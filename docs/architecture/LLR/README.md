# Low Level Requirements (LLR)

**Project:** License Plate Reader with ESP32
**Version:** 1.0
**Date:** 2026-03-18
**Authors:** Andrea Venegas, Sergio Lara

---

## 1. Hardware Mapping

### 1.1 GPIO Pinout

| Function | GPIO Pin | Mode | Voltage | Purpose |
|---|---|---|---|---|
| **LED Status** | GPIO 2 | Output | 3.3V | System state indicator |
| **ADC Input** | GPIO 34 | Input | 3.3V | Analog sensor (thermistor) |
| **RX (Serial)** | GPIO 3 | Input | 3.3V | USB Serial debug |
| **TX (Serial)** | GPIO 1 | Output | 3.3V | USB Serial debug |
| **WiFi** | Internal | — | — | Integrated ESP32 |

### 1.2 Hardware Specifications

| Component | Specification | Notes |
|---|---|---|
| **MCU** | ESP32 (Xtensa dual-core, 240 MHz) | Espressif Systems |
| **Flash** | 4 MB SPI | Firmware + SPIFFS |
| **SRAM** | 520 KB | Image bufers limit: 400 KB |
| **WiFi** | 802.11 b/g/n 2.4 GHz | Max 150 Mbps (not limiting) |
| **Serial** | UART 0, 115200 baud | D0=GPIO1(TX), D1=GPIO3(RX) |
| **Power Input** | USB 5V, 500 mA | Via CP2102 USB-to-Serial |

---

## 2. Software Architecture (Module Level)

### 2.1 Main Modules

```
firmware/
├── boot/
│   └── src/
│       └── main.cpp          ← Entry point, initialization loop
├── appl/
│   ├── bluetooth/ (reserved)
│   ├── wifi/
│   │   ├── inc/WiFiManager.h
│   │   └── src/WiFiManager.cpp
│   ├── image/
│   │   ├── inc/ImageBuffer.h
│   │   ├── inc/ImageProcessor.h
│   │   ├── src/ImageBuffer.cpp
│   │   └── src/ImageProcessor.cpp
│   ├── ocr/
│   │   ├── inc/OCREngine.h
│   │   └── src/OCREngine.cpp  (Python companion interface)
│   ├── led/
│   │   ├── inc/LEDController.h
│   │   └── src/LEDController.cpp
│   └── comm/
│       ├── inc/ResultTransmitter.h
│       └── src/ResultTransmitter.cpp
└── platformio.ini            ← Build configuration
```

---

## 3. Component Specifications

### 3.1 WiFiManager (LLR-01)

**File:** `firmware/appl/wifi/WiFiManager.cpp`

**Public API:**
```cpp
class WiFiManager {
public:
    bool initSoftAP();          // Start AP mode
    bool waitForConnection();   // Wait for client
    bool isConnected();         // Check connection status
    IPAddress getLocalIP();     // Get ESP32 IP
};
```

**Configuration:**
```cpp
const char* SSID = "LicensePlateReader";
const char* PASSWORD = "";                 // Open network for MVP
const uint16_t PORT = 5000;                // TCP listen port
const int MAX_CLIENTS = 1;                 // Single client at a time
```

**Dependencies:**
- `#include <WiFi.h>` (Arduino ESP32 core)
- `#include <WebServer.h>` (Arduino built-in)

---

### 3.2 ImageBuffer (LLR-02)

**File:** `firmware/appl/image/ImageBuffer.cpp`

**Data Structure:**
```cpp
struct ImageBuffer {
    uint8_t* data;              // Raw JPEG bytes
    uint32_t size;              // Bytes received
    uint32_t max_size;          // 400 KB limit
    ImageFormat format;         // JPEG or PNG
    bool ready;                 // Ready for processing
};
```

**Public API:**
```cpp
class ImageBuffer {
public:
    bool receiveImage(WiFiClient& client, uint32_t size_bytes);
    bool isReady();
    uint8_t* getData();
    uint32_t getSize();
    void clear();
};
```

**Memory:**
- Static buffer: `uint8_t imageData[400 * 1024];` in SRAM

---

### 3.3 ImageProcessor (LLR-03)

**File:** `firmware/appl/image/ImageProcessor.cpp`

**Processing Steps:**
1. Decode JPEG → grayscale (via external library or Python)
2. Apply Gaussian blur (3x3 kernel)
3. Binary threshold (adaptive or fixed)
4. Contour detection (find plate region)
5. Crop bounding box

**Public API:**
```cpp
class ImageProcessor {
public:
    bool preprocessImage(uint8_t* jpeg_data, uint32_t jpeg_size);
    bool detectPlateRegion();    // Returns bounding box
    uint8_t* getCroppedPlate();  // Returns cropped region
    BoundingBox getPlateBox();
};

struct BoundingBox {
    uint16_t x, y, width, height;
};
```

**Note:** Heavy lifting (JPEG decode, blur) may be deferred to Python companion via TCP

---

### 3.4 OCREngine (LLR-04)

**File:** `firmware/appl/ocr/OCREngine.cpp`

**Architecture:** ESP32 sends cropped plate image to Python companion script over TCP

**Protocol:**
```
ESP32 → PC (Python):
  POST /ocr
  Content-Type: image/raw
  [cropped_image_bytes]

PC (Python) → ESP32:
  JSON: {"plate": "ABC1234", "confidence": 0.92}
```

**Public API:**
```cpp
class OCREngine {
public:
    bool connectToPythonCompanion(const char* server_ip, uint16_t port);
    bool sendPlateImage(uint8_t* image_data, uint32_t size);
    bool getResult(String& plate_text, float& confidence);
    void disconnectPython();
};
```

**Companion Script Location:**
- `tools/scripts/ocr_server.py` (to be created)
- Runs on laptop during development
- Uses: OpenCV + Tesseract

---

### 3.5 LEDController (LLR-05)

**File:** `firmware/appl/led/LEDController.cpp`

**GPIO:** Pin 2 (GPIO_NUM_2)

**State Machine:**
```
States:
  IDLE        → LED OFF
  PROCESSING  → LED ON (solid)
  DONE        → LED BLINK (500 ms on, 500 ms off) @ 1 Hz
  ERROR       → LED BLINK (100 ms on, 100 ms off) @ 5 Hz
```

**Public API:**
```cpp
enum LEDState { IDLE, PROCESSING, DONE, ERROR };

class LEDController {
public:
    bool init();
    void setState(LEDState state);
    void update();  // Call in main loop (non-blocking)
};
```

**Implementation:** Software timer + millis() for blinking (no FreeRTOS task overhead)

---

### 3.6 ResultTransmitter (LLR-06)

**File:** `firmware/appl/comm/ResultTransmitter.cpp`

**Response Format (JSON):**
```json
{
  "status": "success",
  "plate": "ABC1234",
  "confidence": 0.92,
  "timestamp": 1234567890
}
```

**Error Response:**
```json
{
  "status": "error",
  "message": "OCR failed: low confidence",
  "code": 500
}
```

**Public API:**
```cpp
class ResultTransmitter {
public:
    bool sendResult(WiFiClient& client, const String& plate, float confidence);
    bool sendError(WiFiClient& client, const String& message);
};
```

**Dependencies:**
- `#include <ArduinoJson.h>` (ArduinoJson library v6.x)

---

## 4. System Initialization Sequence (LLR-07)

**In main.cpp:**

```
BOOT
  │
  ├─► setup()
  │    ├─ Serial.begin(115200)
  │    ├─ LEDController.init()        → LED OFF
  │    ├─ WiFiManager.initSoftAP()    → Start WiFi AP
  │    ├─ Print WiFi SSID + IP to Serial
  │    └─ Set LED → IDLE
  │
  └─► loop()
       ├─ WiFiManager.waitForConnection()  → Blocks until client connects
       ├─ Set LED → PROCESSING
       ├─ ImageBuffer.receiveImage()
       ├─ ImageProcessor.preprocessImage()
       ├─ ImageProcessor.detectPlateRegion()
       ├─ OCREngine.sendPlateImage()       → To Python
       ├─ OCREngine.getResult()
       ├─ ResultTransmitter.sendResult()   → To client
       ├─ Set LED → DONE (blink 2 s)
       ├─ Clear buffers
       └─ Return to WiFiManager.waitForConnection()
```

---

## 5. Communication Protocols (LLR-08)

### 5.1 Client → ESP32 (WiFi TCP)

```
Endpoint: ESP32_IP:5000/upload
Method: POST
Content-Type: application/octet-stream
Body: Raw JPEG bytes

Response: 200 OK
Content-Type: application/json
{
  "status": "success",
  "plate": "ABC1234",
  "confidence": 0.92
}
```

### 5.2 ESP32 → Python Companion (TCP)

```
Endpoint: PYTHON_IP:5000/ocr
Method: POST
Body: Raw cropped image bytes
Timeout: 2 seconds

Response: 200 OK
{
  "plate": "ABC1234",
  "confidence": 0.92
}
```

### 5.3 Debug → Serial Monitor

```
[BOOT] LicensePlateReader v1.0
[WiFi] AP started: LicensePlateReader (192.168.4.1)
[WiFi] Client connected from 192.168.4.2
[Image] Received 45328 bytes
[Image] Processing plate region...
[OCR] Sending to Python companion at 192.168.1.100:5000
[OCR] Result: ABC1234 (confidence: 0.92)
[Result] Sent to client
[LED] State: DONE (blinking)
```

---

## 6. Build Configuration (LLR-09)

**File:** `firmware/platformio.ini`

```ini
[platformio]
default_envs = esp32dev

[env:esp32dev]
platform = espressif32
board = esp32doit-devkit-v1
framework = arduino

lib_deps =
    ArduinoJson @ ^6.21.0

upload_speed = 921600
monitor_speed = 115200
build_flags =
    -DARDUINO_LOG_LEVEL=4
```

---

## 7. Memory Budget (LLR-10)

| Component | Size | Comments |
|---|---|---|
| **Firmware code** | ~200 KB | Compiled .elf |
| **SPIFFS (file system)** | ~1 MB | Reserved, not used in MVP |
| **SRAM dynamic** | ~100 KB | Stack + heap for objects |
| **ImageBuffer** | 400 KB | SRAM static allocation |
| **Available free** | ~20 KB | Margin for runtime |
| **Total SRAM** | 520 KB | ✅ Fits within budget |

---

## 8. Dependencies & Libraries

| Library | Version | Purpose | Type |
|---|---|---|---|
| Arduino ESP32 Core | 2.x+ | Platform SDK | External |
| ArduinoJson | 6.21+ | JSON parsing | External |
| (OpenCV) | 4.x | Image processing | Python only |
| (Tesseract) | 5.x | OCR | Python only |

---

## 9. Traceability to HLR

| LLR Module | Related HLR |
|---|---|
| LLR-01 (WiFiManager) | HLR-01 |
| LLR-02 (ImageBuffer) | HLR-02 |
| LLR-03 (ImageProcessor) | HLR-03 |
| LLR-04 (OCREngine) | HLR-04 |
| LLR-05 (LEDController) | HLR-05 |
| LLR-06 (ResultTransmitter) | HLR-06 |

---

**Status:** Complete — Ready for implementation
**Next Step:** Create firmware files following this specification
