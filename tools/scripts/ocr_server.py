#!/usr/bin/env python3
"""
License Plate Reader - OCR Companion Server
Runs on laptop, receives images from ESP32 via WiFi, performs OCR.

Usage:
    pip install opencv-python pytesseract flask numpy
    python ocr_server.py

The ESP32 sends images to this server at port 5001.
Connect your laptop to the "LicensePlateReader" WiFi network first.
"""

import re
import sys
import time
import json
import logging
import urllib.request
import urllib.error
from io import BytesIO

try:
    import cv2
    import numpy as np
except ImportError:
    print("ERROR: OpenCV not installed. Run: pip install opencv-python numpy")
    sys.exit(1)

try:
    import pytesseract
except ImportError:
    print("ERROR: pytesseract not installed. Run: pip install pytesseract")
    print("Also install Tesseract OCR: brew install tesseract (macOS)")
    sys.exit(1)

from http.server import HTTPServer, BaseHTTPRequestHandler

# Configuration
HOST = "0.0.0.0"
PORT = 5001
MAX_IMAGE_SIZE = 500 * 1024  # 500 KB

logging.basicConfig(
    level=logging.INFO,
    format="[%(asctime)s] %(levelname)s: %(message)s",
    datefmt="%H:%M:%S"
)
log = logging.getLogger("OCR")


def preprocess_image(image_bytes):
    """Decode image bytes to OpenCV image."""
    nparr = np.frombuffer(image_bytes, np.uint8)
    img = cv2.imdecode(nparr, cv2.IMREAD_COLOR)
    if img is None:
        log.error("Failed to decode image")
        return None
    log.info(f"Image decoded: {img.shape[1]}x{img.shape[0]}")
    return img


def detect_plate_region(img):
    """
    Try to isolate the license plate rectangle.
    Falls back to the full image if nothing is found.
    """
    h, w = img.shape[:2]

    # Work on a copy scaled to max 1024px wide (faster + more stable)
    scale = min(1.0, 1024.0 / w)
    work = cv2.resize(img, None, fx=scale, fy=scale)
    wh, ww = work.shape[:2]

    gray = cv2.cvtColor(work, cv2.COLOR_BGR2GRAY)
    filtered = cv2.bilateralFilter(gray, 13, 15, 15)
    edged = cv2.Canny(filtered, 30, 180)

    # Dilate edges slightly to close small gaps
    kernel = cv2.getStructuringElement(cv2.MORPH_RECT, (3, 1))
    edged = cv2.dilate(edged, kernel, iterations=1)

    contours, _ = cv2.findContours(edged, cv2.RETR_TREE, cv2.CHAIN_APPROX_SIMPLE)
    contours = sorted(contours, key=cv2.contourArea, reverse=True)[:50]

    candidates = []
    for cnt in contours:
        peri = cv2.arcLength(cnt, True)
        approx = cv2.approxPolyDP(cnt, 0.018 * peri, True)
        if len(approx) == 4:
            x, y, bw, bh = cv2.boundingRect(approx)
            ar = bw / float(bh)
            # Plates typically 2:1 to 5.5:1; must be a reasonable size
            if 1.8 < ar < 5.5 and bw > ww * 0.1 and bh > wh * 0.03:
                candidates.append((x, y, bw, bh, bw * bh))

    if candidates:
        candidates.sort(key=lambda c: c[4], reverse=True)
        x, y, bw, bh, _ = candidates[0]
        # Map back to original resolution
        x = int(x / scale); y = int(y / scale)
        bw = int(bw / scale); bh = int(bh / scale)
        pad_x = int(bw * 0.03); pad_y = int(bh * 0.06)
        x = max(0, x - pad_x); y = max(0, y - pad_y)
        bw = min(w - x, bw + 2 * pad_x); bh = min(h - y, bh + 2 * pad_y)
        roi = img[y:y+bh, x:x+bw]
        log.info(f"Plate region: ({x},{y}) {bw}x{bh}")
        return roi

    log.info("No plate region found, using full image")
    return img


def get_image_crops(img):
    """
    Return a list of (crop, label) for OCR attempts.
    Covers: full image, center strip (number row), bottom half, top half.
    Mexican plates: state name top ~25%, number middle ~55%, strip bottom ~20%.
    """
    if len(img.shape) == 3:
        gray = cv2.cvtColor(img, cv2.COLOR_BGR2GRAY)
    else:
        gray = img.copy()

    h, w = gray.shape
    crops = []
    crops.append((gray,                          "full"))
    crops.append((gray[int(h*0.20):int(h*0.85), :], "center"))
    crops.append((gray[int(h*0.30):int(h*0.80), :], "number_strip"))
    crops.append((gray[int(h*0.45):          :, :], "bottom_half"))
    # Add crop focused on the tallest characters (ignores small state-name text)
    main_row = find_main_char_row(gray)
    if main_row.shape[0] != gray.shape[0]:  # only add if it's actually a sub-crop
        crops.insert(0, (main_row, "main_chars"))
    return crops


def prepare_variants(crop):
    """
    Given a grayscale crop, return a list of (processed_image, label) ready
    for Tesseract. Upscales to at least 80px tall.
    """
    h, w = crop.shape

    # Upscale aggressively — Tesseract needs characters ~40-60px tall
    target_h = 200
    if h < target_h:
        scale = target_h / h
        crop = cv2.resize(crop, None, fx=scale, fy=scale,
                          interpolation=cv2.INTER_CUBIC)

    # Gentle denoise
    denoised = cv2.GaussianBlur(crop, (3, 3), 0)

    variants = []

    # V1: Otsu
    _, v1 = cv2.threshold(denoised, 0, 255, cv2.THRESH_BINARY + cv2.THRESH_OTSU)
    variants.append((v1, "otsu"))

    # V2: Otsu inverted
    variants.append((cv2.bitwise_not(v1), "otsu_inv"))

    # V3: Adaptive
    v3 = cv2.adaptiveThreshold(denoised, 255, cv2.ADAPTIVE_THRESH_GAUSSIAN_C,
                                cv2.THRESH_BINARY, 15, 8)
    variants.append((v3, "adapt"))

    # V4: Adaptive inverted
    variants.append((cv2.bitwise_not(v3), "adapt_inv"))

    # V5: Morphology + Otsu
    k = cv2.getStructuringElement(cv2.MORPH_RECT, (2, 2))
    cleaned = cv2.morphologyEx(denoised, cv2.MORPH_OPEN, k)
    _, v5 = cv2.threshold(cleaned, 0, 255, cv2.THRESH_BINARY + cv2.THRESH_OTSU)
    variants.append((v5, "morph"))

    return variants


def run_tesseract(img, config):
    """Run Tesseract and return (text, confidence). Returns ('', 0) on failure."""
    try:
        data = pytesseract.image_to_data(img, config=config, output_type=pytesseract.Output.DICT)
        words = []
        confs = []
        for i, word in enumerate(data["text"]):
            c = int(data["conf"][i])
            word = word.strip()
            if c > 0 and word:
                words.append(word)
                confs.append(c)
        text = "".join(words).upper()
        text = "".join(ch for ch in text if ch.isalnum())
        avg_conf = sum(confs) / len(confs) / 100.0 if confs else 0.0
        return text, avg_conf
    except Exception as e:
        log.debug(f"Tesseract error: {e}")
        return "", 0.0


def fix_plate(text):
    """
    Apply position-aware OCR correction for Mexican plates.
    Format: AAA000A (3 letters, 3 digits, 1 optional letter)
    Also handles AAA000 (6 chars).
    Only corrects chars in the digit positions (3,4,5).
    """
    if len(text) < 6:
        return text

    # Digit position corrections: chars that look like digits but OCR reads as letters
    digit_fixes = {'O': '0', 'N': '0', 'D': '0', 'U': '0', 'Q': '0',
                   'I': '1', 'L': '1', 'Z': '2', 'S': '5', 'G': '6', 'B': '8'}
    # Letter position corrections: digits that look like letters
    letter_fixes = {'0': 'O', '1': 'I', '5': 'S', '8': 'B', '6': 'G', '2': 'Z'}

    chars = list(text)

    # Positions 0,1,2 should be letters
    for i in range(min(3, len(chars))):
        if chars[i] in letter_fixes:
            chars[i] = letter_fixes[chars[i]]

    # Positions 3,4,5 should be digits
    for i in range(3, min(6, len(chars))):
        if chars[i] in digit_fixes:
            chars[i] = digit_fixes[chars[i]]

    # Position 6 (if present) should be a letter
    if len(chars) > 6 and chars[6] in letter_fixes:
        chars[6] = letter_fixes[chars[6]]

    return "".join(chars)


def find_main_char_row(gray):
    """
    Use connected-component analysis to find the horizontal band that contains
    the LARGEST characters (the plate number). Small text like state names and
    footer strips are filtered out because their character height is much smaller.
    Returns a cropped grayscale image of that band.
    """
    h, w = gray.shape

    # Binarize
    blurred = cv2.GaussianBlur(gray, (3, 3), 0)
    _, binary = cv2.threshold(blurred, 0, 255, cv2.THRESH_BINARY_INV + cv2.THRESH_OTSU)

    num_labels, labels, stats, _ = cv2.connectedComponentsWithStats(binary, connectivity=8)

    if num_labels < 2:
        return gray  # nothing found, return as-is

    # Collect heights of all non-background components
    all_heights = stats[1:, cv2.CC_STAT_HEIGHT]  # skip label 0 (background)

    if len(all_heights) == 0:
        return gray

    max_h = int(np.max(all_heights))

    # Keep only components whose height is >= 40% of the tallest one
    min_h = max(4, int(max_h * 0.40))

    top_row = h
    bot_row = 0
    for label_idx in range(1, num_labels):
        lh = stats[label_idx, cv2.CC_STAT_HEIGHT]
        if lh >= min_h:
            ly = stats[label_idx, cv2.CC_STAT_TOP]
            top_row = min(top_row, ly)
            bot_row = max(bot_row, ly + lh)

    if bot_row <= top_row:
        return gray

    # Add 10% vertical padding
    pad = max(2, int((bot_row - top_row) * 0.10))
    top_row = max(0, top_row - pad)
    bot_row = min(h, bot_row + pad)

    log.info(f"Main char row: y={top_row}..{bot_row} (from {h}px total)")
    return gray[top_row:bot_row, :]


def extract_plate_pattern(text):
    """
    Given raw OCR text (may include state name like 'JALISCO', stickers, etc.),
    extract the most likely plate number using regex.
    Mexican federal format: AAA-000-A  (3 letters, 3 digits, 1 letter = 7 chars)
    Also handles older formats and 6-char plates.
    """
    if not text:
        return text

    # Priority 1: 7-char pattern (AAA000A)
    m = re.search(r'[A-Z]{2,3}[0-9]{2,3}[A-Z]{1,2}', text)
    if m:
        return m.group(0)

    # Priority 2: 6-char pattern (AAA000 or AA0000)
    m = re.search(r'[A-Z]{2,3}[0-9]{2,4}', text)
    if m:
        return m.group(0)

    # Priority 3: any 5-8 alphanumeric chars that contain both letters and digits
    for m in re.finditer(r'[A-Z0-9]{5,8}', text):
        candidate = m.group(0)
        has_letter = any(c.isalpha() for c in candidate)
        has_digit = any(c.isdigit() for c in candidate)
        if has_letter and has_digit:
            return candidate

    return text


def perform_ocr(plate_img):
    """
    Run OCR with multiple crops, image variants and Tesseract configs.
    Returns (best_text, best_confidence).
    """
    # PSM 7  = single text line
    # PSM 8  = single word
    # PSM 6  = uniform block of text (handles multi-line plates well)
    # PSM 13 = raw line, no layout analysis
    configs = [
        "--oem 3 --psm 7  -c tessedit_char_whitelist=ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789",
        "--oem 3 --psm 8  -c tessedit_char_whitelist=ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789",
        "--oem 3 --psm 6  -c tessedit_char_whitelist=ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789",
        "--oem 3 --psm 13 -c tessedit_char_whitelist=ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789",
        "--oem 3 --psm 6",   # no whitelist fallback
        "--oem 3 --psm 7",
    ]

    best_text = ""
    best_conf = 0.0
    best_score = 0.0

    crops = get_image_crops(plate_img)

    for crop, crop_label in crops:
        variants = prepare_variants(crop)
        for img, var_label in variants:
            for config in configs:
                text, conf = run_tesseract(img, config)

                # Must be at least 3 alphanumeric chars
                if len(text) < 3:
                    continue

                # Score: confidence * length bonus (plates are 5-8 chars)
                # Penalize texts > 9 chars — likely picked up state name or extra garbage
                ideal_len = 7  # AAA000A
                if len(text) <= ideal_len:
                    length_factor = len(text) / ideal_len
                else:
                    # Penalty for extra characters beyond 9
                    length_factor = max(0.0, 1.0 - (len(text) - 9) * 0.15)
                score = conf * length_factor

                if score > 0:
                    log.info(f"  [{crop_label}/{var_label}] '{text}' conf={conf:.2f} score={score:.3f}")

                if score > best_score:
                    best_text = text
                    best_conf = conf
                    best_score = score

    if best_text:
        best_text = extract_plate_pattern(best_text)
        best_text = fix_plate(best_text)
        log.info(f"Best: '{best_text}' conf={best_conf:.2f}")
        return best_text, best_conf

    log.warning("OCR returned no valid result")
    return "", 0.0
class OCRHandler(BaseHTTPRequestHandler):
    """HTTP handler for OCR requests from ESP32."""

    def log_message(self, format, *args):
        log.info(format % args)

    def do_POST(self):
        if self.path != "/ocr":
            self.send_error(404, "Not found")
            return

        content_length = int(self.headers.get("Content-Length", 0))
        if content_length == 0 or content_length > MAX_IMAGE_SIZE:
            self.send_error(400, "Invalid content length")
            return

        # Read image data
        image_bytes = self.rfile.read(content_length)
        log.info(f"Received {len(image_bytes)} bytes from ESP32")

        # Save raw image for debugging
        debug_path = "/tmp/lpr_debug_raw.jpg"
        with open(debug_path, "wb") as f:
            f.write(image_bytes)
        log.info(f"Debug image saved to {debug_path}")

        start_time = time.time()

        # Process image
        result = preprocess_image(image_bytes)
        if result is None:
            response = json.dumps({"plate": "", "confidence": 0.0, "error": "decode_failed"})
            self.send_response(200)
            self.send_header("Content-Type", "application/json")
            self.end_headers()
            self.wfile.write(response.encode())
            return

        # Detect plate region
        plate_image = detect_plate_region(result)

        # Perform OCR
        plate_text, confidence = perform_ocr(plate_image)

        elapsed = time.time() - start_time
        log.info(f"OCR result: '{plate_text}' (confidence: {confidence:.2f}) in {elapsed:.2f}s")

        # Send response
        response = json.dumps({
            "plate": plate_text,
            "confidence": round(confidence, 3),
            "processing_time_ms": round(elapsed * 1000)
        })

        self.send_response(200)
        self.send_header("Content-Type", "application/json")
        self.send_header("Content-Length", str(len(response)))
        self.end_headers()
        self.wfile.write(response.encode())

    def do_GET(self):
        if self.path == "/health":
            response = json.dumps({"status": "ok", "tesseract": True})
            self.send_response(200)
            self.send_header("Content-Type", "application/json")
            self.end_headers()
            self.wfile.write(response.encode())
        else:
            self.send_error(404, "Not found")


def register_with_esp32(esp32_ip="192.168.4.1", esp32_port=5000, retries=5):
    """Tell the ESP32 our IP so it knows where to send images for OCR."""
    url = f"http://{esp32_ip}:{esp32_port}/register"
    for attempt in range(1, retries + 1):
        try:
            req = urllib.request.Request(url, data=b"", method="POST")
            with urllib.request.urlopen(req, timeout=3) as resp:
                body = resp.read().decode()
                log.info(f"Registered with ESP32: {body}")
                return True
        except Exception as e:
            log.warning(f"Register attempt {attempt}/{retries} failed: {e}")
            if attempt < retries:
                time.sleep(2)
    log.error("Could not register with ESP32. Make sure you are connected to LicensePlateReader WiFi.")
    return False


def main():
    print("=" * 50)
    print("  License Plate Reader - OCR Server")
    print("=" * 50)
    print(f"  Listening on {HOST}:{PORT}")
    print(f"  Tesseract: {pytesseract.get_tesseract_version()}")
    print()
    print("  Instructions:")
    print("  1. Connect to 'LicensePlateReader' WiFi")
    print("  2. This server receives images from ESP32")
    print("  3. Press Ctrl+C to stop")
    print("=" * 50)

    server = HTTPServer((HOST, PORT), OCRHandler)

    # Register our IP with the ESP32
    register_with_esp32()

    try:
        server.serve_forever()
    except KeyboardInterrupt:
        print("\nServer stopped.")
        server.server_close()


if __name__ == "__main__":
    main()
