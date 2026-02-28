# Overview — License Plate Reader (ESP32)

---

## Project Description

The **License Plate Reader** is an embedded system based on the ESP32 microcontroller that captures vehicle license plate images (sent wirelessly from a mobile phone) and extracts the alphanumeric text from the plate using computer vision and Optical Character Recognition (OCR) techniques.

The system processes the received image, locates the plate region, and returns the recognized characters to the user — all without requiring expensive dedicated hardware, making it practical and accessible for parking control or vehicle identification applications.

---

## Objective

Design and implement an embedded system that:
- Receives a vehicle license plate image via WiFi from a mobile device.
- Detects and segments the plate region using image processing (OpenCV).
- Extracts the plate text using OCR (Tesseract).
- Displays or transmits the result back to the user in real time.

---

## Project Feasibility

The ESP32 is technically suitable for this project due to the following reasons:

| Factor | Justification |
|---|---|
| **WiFi connectivity** | Built-in 802.11 b/g/n — no additional networking hardware needed. |
| **Processing power** | Dual-core Xtensa LX6 @ 240 MHz with sufficient RAM for lightweight ML inference. |
| **TensorFlow Lite support** | TFLite Micro is specifically optimized for microcontrollers like the ESP32. |
| **Open-source toolchain** | Arduino IDE / VS Code + PlatformIO; OpenCV and Tesseract are free and well-documented. |
| **Cost** | The ESP32 module costs ~$5–10 USD; the mobile phone camera reuses existing hardware. |
| **Timeline** | The client-server WiFi architecture decouples processing so it can be prototyped incrementally. |

---

## Hardware Used

| Component | Role | Justification |
|---|---|---|
| **ESP32** | Main microcontroller | WiFi, dual-core processing, TFLite compatibility |
| **Mobile phone (camera)** | Image capture | Avoids dedicated camera module; high resolution over WiFi |
| **Protoboard + jumper wires** | Prototyping platform | Rapid, reusable connections |
| **LED indicator** | Status feedback | Visual confirmation of processing state |
| **Resistors (220 Ohm, 10 kOhm)** | Circuit protection / pull-down | Required for LED and sensor circuits |
| **USB cable** | Programming & power | Flashing firmware via serial |

---

## Architecture & System Requirements

The full system requirements are documented in the SRS:
[docs/architecture/SRS/README.md](../docs/architecture/SRS/README.md)

High-level and low-level requirements:
[docs/architecture/HLR/README.md](../docs/architecture/HLR/README.md)
[docs/architecture/LLR/README.md](../docs/architecture/LLR/README.md)

---

## Team Members & Roles

| Member | Product Owner | Scrum Master | Dev — Hardware | Dev — Firmware | Dev — Integration | Dev — Testing |
|---|:---:|:---:|:---:|:---:|:---:|:---:|
| **Andrea Venegas** | | ✓ | ✓ | | ✓ | ✓ |
| **Sergio Lara** | ✓ | | | ✓ | ✓ | ✓ |

Full Project Vision, Sprint Planning, Backlog, DoR/DoD → [`docs/project-vision.md`](../docs/project-vision.md)
