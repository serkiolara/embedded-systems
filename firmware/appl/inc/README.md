# firmware/appl/inc

Shared header files used across application modules.

Each application module (`wifi`, `image`, `ocr`, `led`, `comm`) keeps its own `inc/` folder with its public header. This folder is reserved for headers that are shared between multiple modules.

See each module's own `inc/` for its specific API:

| Module | Header | Purpose |
|---|---|---|
| `wifi/` | `WiFiManager.h` | SoftAP setup and client tracking |
| `image/` | `ImageBuffer.h` | 60 KB JPEG receive buffer |
| `ocr/` | `OCREngine.h` | HTTP client to Python OCR server |
| `led/` | `LEDController.h` | GPIO2 state machine (IDLE/PROCESSING/DONE/ERROR) |
| `comm/` | `ResultTransmitter.h` | Result forwarding utilities |
