# High Level Requirements (HLR)

**Project:** License Plate Reader with ESP32
**Version:** 1.0
**Date:** 2026-03-18
**Authors:** Andrea Venegas, Sergio Lara

---

## 1. System Architecture Overview

The License Plate Reader is composed of **4 main architectural blocks**:

```
┌─────────────────────────────────────────────────────────────┐
│                     License Plate Reader System              │
├─────────────────────────────────────────────────────────────┤
│                                                              │
│  ┌──────────────────┐         ┌──────────────────────────┐ │
│  │  WiFi Manager    │◄───────►│  Image Reception (TCP)   │ │
│  │  (F-01)          │         │  (F-02)                  │ │
│  └──────────────────┘         └──────────────────────────┘ │
│           │                             │                    │
│           └─────────────┬───────────────┘                    │
│                         │                                    │
│                 ┌───────▼───────┐                            │
│                 │ Image Pipeline│                            │
│                 │ Processing    │                            │
│                 │ (F-03, F-04)  │                            │
│                 └───────┬───────┘                            │
│                         │                                    │
│         ┌───────────────┼───────────────┐                   │
│         │               │               │                   │
│    ┌────▼──┐    ┌──────▼──────┐  ┌────▼──────┐             │
│    │OCR    │    │LED State    │  │Result     │             │
│    │Engine │    │Machine      │  │Transmitter│             │
│    │(F-05) │    │(F-07)       │  │(F-06)     │             │
│    └───────┘    └─────────────┘  └───────────┘             │
│                                                              │
└─────────────────────────────────────────────────────────────┘
```

---

## 2. Architectural Blocks

### 2.1 WiFi Manager (HLR-01)
**Purpose:** Establish and maintain WiFi connectivity
**Responsibility:**
- Initialize ESP32 WiFi hardware
- Create soft AP mode (SSID: "LicensePlateReader", no password for MVP)
- Listen for incoming TCP connections on port 5000/8000
- Handle client connection/disconnection

**Input:** None (initialization on boot)
**Output:** Active TCP listener ready to accept image data
**Dependencies:** Arduino WiFi library, ESP32 core

---

### 2.2 Image Reception over TCP (HLR-02)
**Purpose:** Receive JPEG/PNG image data from mobile phone
**Responsibility:**
- Accept TCP connection from client
- Receive image file in chunks (up to 500 KB)
- Buffer image into RAM or SPIFFS
- Validate image format

**Input:** Raw JPEG bytes from WiFi socket
**Output:** Image stored in memory, ready for processing
**Dependencies:** WiFi socket API, memory management

---

### 2.3 Image Processing Pipeline (HLR-03)
**Purpose:** Prepare image for OCR by detecting license plate region
**Responsibility:**
- Convert RGB/JPEG to grayscale
- Apply noise reduction (Gaussian blur, morphology)
- Detect plate contours or use TFLite model
- Crop plate region
- Send to OCR engine

**Input:** Raw image from TCP reception
**Output:** Cropped plate region (binary or grayscale)
**Dependencies:** OpenCV (C++ or via Python companion), TFLite

---

### 2.4 OCR Engine (HLR-04)
**Purpose:** Extract alphanumeric text from plate image
**Responsibility:**
- Receive cropped plate region
- Run Tesseract OCR or Python script on companion device
- Return plate string (e.g., "ABCD1234")
- Handle OCR failures gracefully

**Input:** Cropped plate image
**Output:** Recognized plate string + confidence
**Dependencies:** Tesseract OCR, Python companion (for MVP)

---

### 2.5 LED State Machine (HLR-05)
**Purpose:** Provide visual feedback to user
**Responsibility:**
- Manage LED GPIO states
- Idle: OFF
- Processing: Solid ON
- Result ready: Slow blink (0.5 Hz)
- Error: Fast blink (2 Hz)

**Input:** Current system state (idle, processing, done, error)
**Output:** LED GPIO signal
**Dependencies:** GPIO driver, timer

---

### 2.6 Result Transmitter (HLR-06)
**Purpose:** Send OCR result back to client
**Responsibility:**
- Format result as JSON: `{"plate": "ABC1234", "confidence": 0.92}`
- Send via TCP to requesting client
- Log result to Serial for debugging

**Input:** Recognized plate string + confidence
**Output:** TCP response to client
**Dependencies:** WiFi socket API, JSON library (ArduinoJson)

---

## 3. Data Flow

```
Mobile Phone                          ESP32
    │                                  │
    ├─ "Connect to LicensePlateReader" ─►┐
    │                            WiFi AP │
    │                           (HLR-01) │
    │◄─ Connection ACK ─────────────────┤
    │                                   │
    ├─ POST /img + JPEG data ──────────►┐
    │                           TCP Rcv  │
    │                           (HLR-02) │
    │                                   │
    │                        ┌──────────────────────┐
    │                        │ Image Processing    │
    │                        │ Grayscale, Detect   │
    │                        │ plate region (HLR-03)
    │                        │                    │
    │                        │ OCR Engine         │
    │                        │ Extract text       │
    │                        │ (HLR-04)          │
    │                        │                   │
    │                        │ LED: Processing   │
    │                        │ (HLR-05)          │
    │                        └──────────────────┘
    │                                   │
    │◄──────── JSON Result ────────────┤
    │       {"plate": "ABC1234"}        │
    │                           (HLR-06)
    │                                   │
```

---

## 4. System Constraints (from HLR perspective)

| Constraint | Value | Reason |
|---|---|---|
| **Total BOM Cost** | ≤ $20 USD | Academic project budget |
| **ESP32 RAM** | 520 KB max | Hardware limitation |
| **WiFi Bandwidth** | Standard 802.11 b/g/n | Integrated hardware |
| **Image size** | ≤ 500 KB | Must fit in RAM |
| **End-to-end latency** | ≤ 3 s (target) | User experience |
| **OCR accuracy** | ≥ 70 % (MVP), ≥ 85 % (final) | Functional threshold |

---

## 5. External Interfaces (HLR-level)

| Interface | Type | Details |
|---|---|---|
| **WiFi** | Wireless | 802.11 b/g/n, 2.4 GHz, SoftAP mode |
| **TCP/IP** | Network | Port 5000, REST-like API (POST /img, GET /result) |
| **Serial Monitor** | Debug | 115200 baud, UART over USB |
| **LED GPIO** | Output | GPIO pin (TO BE DEFINED in LLR) |
| **USB Power** | Power | 5V @ 500 mA (USB 2.0) |

---

## 6. Traceability to SRS

| HLR Block | Related SRS FR/NFR |
|---|---|
| HLR-01 (WiFi Manager) | FR-01 |
| HLR-02 (Image Reception) | FR-02 |
| HLR-03 (Image Processing) | FR-03, FR-04 |
| HLR-04 (OCR Engine) | FR-05 |
| HLR-05 (LED State Machine) | FR-07 |
| HLR-06 (Result Transmitter) | FR-06, FR-08 |

---

**Status:** Complete — Ready for review
**Next Step:** Create LLR with specific GPIO pins, libraries, and API details
