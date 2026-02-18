# Software Requirements Specification (SRS)

**Project Name:** License Plate Reader with ESP32  
**Version:** 1.0 (Draft)  
**Date:** 2026-02-17  
**Author(s):** Andrea Venegas, Sergio Lara  
**Responsibles Signs:** Andrea Venegas / Sergio Lara

---

## 1. Introduction

### 1.1 Purpose
This document establishes the system-level requirements for the License Plate Reader embedded system. It defines what the system must do (functional requirements), how it must perform (non-functional requirements), and the interfaces it relies on, serving as the baseline for design, implementation, and verification.

### 1.2 Scope
The system is an ESP32-based embedded device that receives a vehicle license plate image via WiFi from a mobile phone, processes it using computer vision and OCR, and returns the extracted alphanumeric plate text to the user. The system targets parking control and vehicle identification scenarios where low-cost, standalone hardware is preferred.

### 1.3 Definitions, Acronyms, and Abbreviations

| Term | Definition |
|---|---|
| ESP32 | Espressif Systems dual-core 32-bit microcontroller with integrated WiFi/Bluetooth |
| OCR | Optical Character Recognition — software technique to extract text from images |
| OpenCV | Open Source Computer Vision Library |
| TFLite | TensorFlow Lite — lightweight ML inference framework for embedded devices |
| WiFi | IEEE 802.11 wireless local area network protocol |
| ADC | Analog-to-Digital Converter |
| SRS | Software Requirements Specification |
| FR | Functional Requirement |
| NFR | Non-Functional Requirement |

### 1.4 References
- Espressif ESP32 Technical Reference Manual — https://www.espressif.com/sites/default/files/documentation/esp32_technical_reference_manual_en.pdf
- TensorFlow Lite for Microcontrollers — https://www.tensorflow.org/lite/microcontrollers
- OpenCV Documentation — https://docs.opencv.org/
- Tesseract OCR — https://github.com/tesseract-ocr/tesseract
- Arduino ESP32 Core — https://github.com/espressif/arduino-esp32

### 1.5 Document Overview
Section 2 provides a general product description. Section 3 details the specific functional, non-functional, and interface requirements. Sections 4 and 5 (V&V and Traceability Matrix) are not included in this release.

---

## 2. Overall Description

### 2.1 Product Perspective
The License Plate Reader is a standalone embedded system. The mobile phone acts solely as an image capture and display interface; the ESP32 performs all processing locally. No cloud connectivity is required for basic operation, though optional Firebase logging may be added in future releases.

### 2.2 Product Functions
- Accept an image of a vehicle license plate over a WiFi socket from a mobile device.
- Pre-process the image (grayscale conversion, noise reduction, thresholding).
- Detect and segment the license plate region within the image.
- Extract alphanumeric characters from the plate region using OCR.
- Return the recognized plate string to the client device.
- Provide a visual status indicator (LED) for system states: idle, processing, result ready, error.

### 2.3 User Characteristics
The primary users are engineering students and lab evaluators with intermediate knowledge of embedded systems and basic networking. No specialized training is required to operate the system beyond pointing a mobile phone camera at a license plate and triggering the capture via a simple interface.

### 2.4 Constraints
- **Hardware constraints:** The ESP32 has limited RAM (~520 KB SRAM) and flash (4 MB typical); image and model sizes must be managed accordingly.
- **Processing constraint:** Heavy ML inference may require a client-server split (ESP32 as coordinator, phone or PC handles OpenCV/Tesseract).
- **Power:** Powered via USB (5V/500 mA); no battery optimization required for lab prototype.
- **Cost:** Total BOM must remain under $20 USD.
- **Environment:** Indoor use only; controlled lighting for reliable OCR.
- **Regulatory:** No specific regulatory standard required for academic prototype.

### 2.5 Assumptions and Dependencies
- A mobile phone with WiFi capability is available to the user.
- The ESP32 and mobile device are connected to the same local WiFi network or the ESP32 acts as a WiFi access point.
- Tesseract OCR and OpenCV are available either on the ESP32 or on a companion device (Python server on laptop/phone).
- License plates follow a consistent format (Mexican standard plates assumed for prototype).

---

## 3. Specific Requirements

### 3.1 Functional Requirements

**FR-01:** The system shall establish a WiFi connection (station or AP mode) and expose a TCP/HTTP socket to accept incoming image data from a client device.

**FR-02:** The system shall receive a JPEG or PNG image of a license plate over the network connection with a maximum transfer time of 5 seconds.

**FR-03:** The system shall convert the received image to grayscale and apply noise-reduction filtering before further processing.

**FR-04:** The system shall detect the license plate region within the image using contour detection or a trained Haar/TFLite model.

**FR-05:** The system shall extract the cropped plate region and pass it to an OCR engine (Tesseract or equivalent) to obtain the alphanumeric string.

**FR-06:** The system shall return the recognized plate string to the client device over the same network connection within 10 seconds of image receipt.

**FR-07:** The system shall drive a GPIO-connected LED to indicate system state: OFF = idle, solid ON = processing, blinking = result ready, fast blink = error.

**FR-08:** The system shall handle connection errors and image decoding failures gracefully, logging an error message over Serial and returning an error code to the client.

---

### 3.2 Non-Functional Requirements

**NFR-01 — Performance:** End-to-end latency from image receipt to result delivery shall not exceed 10 seconds under normal operating conditions.

**NFR-02 — Power consumption:** The system shall operate within the USB 2.0 power budget (max 500 mA at 5 V = 2.5 W).

**NFR-03 — Reliability / Availability:** The system shall correctly recognize at least 80% of well-lit, non-obstructed license plates during lab demonstration.

**NFR-04 — Security:** No authentication is required for the academic prototype; the system operates only on a local network.

**NFR-05 — Maintainability:** Firmware shall be organized in modular source files (WiFi, image processing, OCR, LED control) to facilitate independent updates and testing.

---

### 3.3 External Interface Requirements

#### 3.3.1 User Interfaces
- **LED indicator** on a GPIO pin: single RGB or standard LED reflecting system state (see FR-07).
- **Serial Monitor** (115200 baud): debug output including ADC readings, processing status, and recognized plate string.
- **Mobile phone UI**: any browser or custom app capable of sending a POST request with an image file to the ESP32's IP address.

#### 3.3.2 Hardware Interfaces
- **GPIO34 (ADC input):** analog sensor input (used in Lab 1 — NTC thermistor).
- **GPIO (LED output):** status LED with 220 Ohm series resistor.
- **WiFi transceiver:** integrated 802.11 b/g/n for image data transfer.
- **USB-to-Serial (CP2102/CH340):** firmware flashing and serial debug.

#### 3.3.3 Software Interfaces
- **Arduino ESP32 Core / PlatformIO:** firmware build environment.
- **OpenCV (Python or C++):** image pre-processing and plate region detection.
- **Tesseract OCR:** character extraction from cropped plate image.
- **TensorFlow Lite (optional):** alternative lightweight inference for plate detection.
- **HTTP/TCP socket:** communication protocol between mobile client and ESP32 server.

---

### 3.4 Real-Time Requirements
- Image data must be fully received within **5 seconds** of connection establishment (FR-02).
- OCR result must be returned to the client within **10 seconds** of image receipt (FR-06).
- LED state must update within **200 ms** of any state transition (FR-07).

---

### 3.5 Safety and Regulatory Requirements
- This is an academic prototype; no formal regulatory certification (CE, FCC, etc.) is required.
- The system operates at 3.3 V logic and 5 V USB power — standard low-voltage safety practices apply (no exposed high-voltage lines).

---

*Sections 4 (Verification and Validation) and 5 (Requirements Traceability Matrix) are not included in this release.*
