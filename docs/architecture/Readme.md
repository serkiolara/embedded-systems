# Architecture

This folder contains the full technical architecture of the License Plate Reader system.

## Structure

| Folder | Content |
|---|---|
| [`HLR/`](HLR/README.md) | High-Level Requirements — architectural blocks, data flow, interfaces |
| [`LLR/`](LLR/README.md) | Low-Level Requirements — GPIO pinout, module API, library dependencies |
| [`SRS/`](SRS/README.md) | Software Requirements Specification — functional and non-functional requirements |

## System Overview

```
 iPhone (browser)
        │  HTTP multipart upload (JPEG ~15 KB)
        ▼
   ESP32 (192.168.4.1:5000)
   ├─ WiFiManager    → SoftAP "LicensePlateReader"
   ├─ WebServer      → serves UI, receives image
   ├─ ImageBuffer    → 60 KB RAM buffer
   ├─ OCREngine      → HTTP POST to Mac
   ├─ LEDController  → GPIO2 status LED
   └─ ResultTransmitter
        │  HTTP POST /ocr (raw JPEG bytes)
        ▼
   Mac (192.168.4.x:5001) — ocr_server.py
   ├─ OpenCV         → plate region detection
   ├─ Tesseract      → character recognition
   └─ JSON response  → {"plate":"ABC123D", "confidence":0.82}
        │
        ▼
   ESP32 renders result page → iPhone displays plate
```

## Design Decisions

- **SoftAP instead of Station mode** — the ESP32 is the network hub; no external router needed.
- **Mac-side OCR** — Tesseract requires too much RAM/flash to run on the ESP32. Splitting compute avoids the MCU memory bottleneck.
- **Client-side compression** — JavaScript on the browser compresses to < 45 KB JPEG before upload so it fits in the 60 KB buffer.
- **IP auto-registration** — Python server POSTs its own IP to `/register` on startup, so no static IP configuration is needed.
