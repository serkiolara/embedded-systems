# Unit Tests

Unit tests for individual firmware modules.

## Modules to test

| Module | Test Focus | Status |
|---|---|---|
| `ImageBuffer` | `clear()`, `setSize()`, overflow protection, `MAX_IMAGE_SIZE` boundary | ⏳ Pending |
| `LEDController` | State transitions, blink interval timing, GPIO output | ⏳ Pending |
| `OCREngine` | JSON parsing, server IP update, timeout handling | ⏳ Pending |
| `WiFiManager` | SoftAP initialization, `getLocalIP()`, `getConnectedClients()` | ⏳ Pending |
| `fix_plate()` (Python) | Position corrections: N→0, O→0, I→1, digit/letter positions | ⏳ Pending |
| `extract_plate_pattern()` (Python) | Regex extraction from noisy OCR text | ⏳ Pending |

## Running Python unit tests

```bash
cd tools/scripts
python -m pytest test_ocr_utils.py -v
```

## Running firmware tests

Firmware unit tests use the PlatformIO native environment (host-side compilation):

```bash
~/.platformio/penv/bin/platformio test -e native
```
