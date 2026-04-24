# Tools & Scripts

## OCR Companion Server — `ocr_server.py`

Python HTTP server that runs on the connected laptop. Receives JPEG images from the ESP32, processes them with OpenCV + Tesseract and returns the recognized plate text as JSON.

### Requirements

```bash
pip install opencv-python pytesseract numpy
brew install tesseract          # macOS
```

### Usage

```bash
# Kill any previous instance and start fresh
lsof -ti :5001 | xargs kill -9 2>/dev/null
python tools/scripts/ocr_server.py
```

1. Connect the laptop to the **LicensePlateReader** WiFi network (password: `lpr12345`).
2. Run the script — it auto-registers its IP with the ESP32 via `POST /register`.
3. Open `http://192.168.4.1:5000` on any device connected to the same network.

### Pipeline

| Step | Function | Description |
|---|---|---|
| 1 | `preprocess_image` | Decode JPEG bytes → OpenCV image |
| 2 | `detect_plate_region` | Bilateral filter + Canny + contour detection to find the plate rectangle |
| 3 | `find_main_char_row` | Connected-component analysis — keeps only tall characters (ignores state name) |
| 4 | `get_image_crops` | Generates 5 crops (full, center, number strip, bottom half, main chars) |
| 5 | `prepare_variants` | Upscale to 200 px height + 5 binarization variants (Otsu, adaptive, morphological) |
| 6 | `perform_ocr` | ~150 Tesseract attempts (crops × variants × PSM configs), best score wins |
| 7 | `extract_plate_pattern` | Regex extracts `AAA000A` pattern, discards garbage like state names |
| 8 | `fix_plate` | Position-aware correction: digits in positions 3-5 (`N→0`, `O→0`, `I→1`…) |

### Ports

| Port | Service |
|---|---|
| `5001` | OCR server (listens for ESP32) |
| `192.168.4.1:5000` | ESP32 web interface |

### Debug

Raw image saved to `/tmp/lpr_debug_raw.jpg` on every request.
