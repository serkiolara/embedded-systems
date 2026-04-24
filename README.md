# License Plate Reader — ESP32

> Sistema embebido de reconocimiento automático de placas vehiculares usando ESP32, OpenCV y Tesseract OCR.

---

## Autores

| Nombre | Matrícula | Rol |
|---|---|---|
| **Andrea Venegas** | 42277 | Scrum Master · Hardware · Integración · Testing |
| **Sergio Lara** | 13475 | Product Owner · Firmware · Integración · Testing |

**Materia:** Sistemas Embebidos — Octavo Semestre  
**Institución:** Universidad del Centro de México (UCQ)

---

## ¿Qué es?

Un prototipo académico de un sistema de reconocimiento de placas vehiculares (LPR — *License Plate Reader*) que funciona sin internet ni hardware costoso. El usuario toma una foto de una placa desde su celular y el sistema devuelve el texto de la placa en pantalla en segundos.

---

## ¿Qué hace?

1. El ESP32 crea su propia red WiFi (`LicensePlateReader`).
2. El usuario abre `http://192.168.4.1:5000` desde su celular y toma una foto de la placa.
3. El celular comprime la imagen (< 45 KB) y la sube al ESP32.
4. El ESP32 reenvía la imagen a la computadora vía HTTP.
5. La computadora procesa la imagen con OpenCV + Tesseract y devuelve el texto.
6. El ESP32 muestra el resultado en la página web del celular.

---

## ¿Cómo funciona?

```
 📱 Celular (browser)
       │
       │  POST /upload  (JPEG ~15 KB)
       ▼
 ┌─────────────────────────────┐
 │       ESP32  192.168.4.1    │
 │  puerto 5000 (Web Server)   │
 │                             │
 │  WiFiManager   → SoftAP     │
 │  ImageBuffer   → 60 KB RAM  │
 │  OCREngine     → HTTP POST  │──────────────────────────┐
 │  LEDController → GPIO2 LED  │                          │
 └─────────────────────────────┘                          │
                                                          ▼
                                         ┌────────────────────────────┐
                                         │   Mac  192.168.4.x:5001    │
                                         │   ocr_server.py            │
                                         │                            │
                                         │  detect_plate_region()     │
                                         │    → OpenCV Canny + contornos
                                         │  find_main_char_row()      │
                                         │    → solo caracteres grandes
                                         │  perform_ocr()             │
                                         │    → ~150 intentos Tesseract
                                         │  extract_plate_pattern()   │
                                         │    → regex AAA000A         │
                                         │  fix_plate()               │
                                         │    → N→0, O→0, I→1…        │
                                         │                            │
                                         │  {"plate":"ABC000D",       │
                                         │   "confidence":0.82}       │
                                         └────────────────────────────┘
```

### LED de estado (GPIO 2 — LED azul integrado)

| Estado | LED |
|---|---|
| Esperando imagen | Apagado |
| Procesando (enviando al Mac) | Encendido fijo |
| Resultado OK | Parpadeo lento 1 Hz |
| Error / sin resultado | Parpadeo rápido 5 Hz |

---

## Tecnologías

| Tecnología | Uso |
|---|---|
| **ESP32 DOIT DevKit V1** | Microcontrolador principal |
| **PlatformIO + Arduino framework** | Compilación y carga del firmware |
| **ArduinoJson 6** | Parseo de la respuesta JSON |
| **Python 3** | Servidor OCR en la computadora |
| **OpenCV** | Procesamiento de imagen y detección de placa |
| **Tesseract 5** | Motor OCR (reconocimiento de caracteres) |
| **pytesseract** | Wrapper Python para Tesseract |

---

## Estructura del proyecto

```
embedded-systems/
├── README.md                    ← este archivo
├── platformio.ini               ← configuración de build (PlatformIO)
│
├── firmware/
│   ├── boot/src/main.cpp        ← punto de entrada ESP32: rutas HTTP, setup, loop
│   └── appl/
│       ├── wifi/                ← WiFiManager: crea el AP WiFi
│       ├── image/               ← ImageBuffer: buffer 60 KB para la imagen
│       ├── ocr/                 ← OCREngine: cliente HTTP al Mac (puerto 5001)
│       ├── led/                 ← LEDController: máquina de estados del LED
│       └── comm/                ← ResultTransmitter: utilidades de reenvío
│
├── tools/scripts/
│   └── ocr_server.py            ← servidor OCR Python (corre en la Mac)
│
├── docs/
│   ├── project-vision.md        ← visión, stakeholders, alcance
│   └── architecture/
│       ├── SRS/                 ← Especificación de Requisitos de Software
│       ├── HLR/                 ← Requisitos de Alto Nivel (bloques arquitectónicos)
│       └── LLR/                 ← Requisitos de Bajo Nivel (GPIO, APIs, librerías)
│
├── hardware/
│   ├── schematics/              ← esquemáticos del circuito
│   └── simulations/             ← simulaciones
│
├── Testing/
│   ├── Unit_tests/              ← pruebas unitarias por módulo
│   └── Integration_Tests/       ← pruebas end-to-end completas
│
└── Overview/                    ← descripción general del proyecto
```

---

## Cómo correr el sistema

### Requisitos

- ESP32 conectado por USB a la Mac (`/dev/cu.usbserial-110`)
- Python 3 con venv en `.venv/`
- Tesseract instalado: `brew install tesseract`

### 1. Subir firmware al ESP32

```bash
cd embedded-systems
~/.platformio/penv/bin/platformio run --target upload
```

### 2. Iniciar servidor OCR en la Mac

```bash
lsof -ti :5001 | xargs kill -9 2>/dev/null
.venv/bin/python tools/scripts/ocr_server.py
```

### 3. Usar el sistema

1. Conectar el celular (o cualquier dispositivo) a la red WiFi **LicensePlateReader** (contraseña: `lpr12345`).
2. Abrir `http://192.168.4.1:5000` en el navegador.
3. Seleccionar o tomar una foto de la placa.
4. Presionar **Analyze** y esperar el resultado (~10–15 s).

---

## Documentación

| Documento | Ruta |
|---|---|
| Visión del proyecto | [`docs/project-vision.md`](docs/project-vision.md) |
| Requisitos (SRS) | [`docs/architecture/SRS/README.md`](docs/architecture/SRS/README.md) |
| Arquitectura (HLR) | [`docs/architecture/HLR/README.md`](docs/architecture/HLR/README.md) |
| Diseño detallado (LLR) | [`docs/architecture/LLR/README.md`](docs/architecture/LLR/README.md) |
| Releases y sprints | [`docs/Releases/SRR/README.md`](docs/Releases/SRR/README.md) |
| Scripts OCR | [`tools/scripts/README.md`](tools/scripts/README.md) |
| Pruebas unitarias | [`Testing/Unit_tests/README.md`](Testing/Unit_tests/README.md) |
| Pruebas de integración | [`Testing/Integration_Tests/README.md`](Testing/Integration_Tests/README.md) |
