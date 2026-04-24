# firmware/boot/src

Main ESP32 entry point. Contains `main.cpp` — the only file in `src_dir` as defined in `platformio.ini`.

## main.cpp responsibilities

| Section | What it does |
|---|---|
| `setup()` | Initializes LED, WiFi AP, OCR engine default IP, registers HTTP routes, starts server |
| `loop()` | Calls `server.handleClient()` + `ledController.update()` every 10 ms |
| `handleRoot()` | Serves the HTML+JS web interface (compression loop, file input, XHR upload) |
| `handleUploadData()` | Receives multipart chunks, appends to `ImageBuffer` |
| `handleUploadComplete()` | Calls `OCREngine::processImage()`, renders result page, updates LED state |
| `handleRegister()` | Updates OCR server IP when Python companion calls `/register` |
| `handleStatus()` | Returns JSON with uptime, free heap, connected clients |

## HTTP Routes

| Method | Path | Handler |
|---|---|---|
| GET | `/` | `handleRoot` — web UI |
| POST | `/upload` | `handleUploadComplete` + `handleUploadData` |
| GET | `/status` | `handleStatus` |
| POST | `/register` | `handleRegister` |
