# Integration Tests

End-to-end tests that validate the full pipeline: iPhone → ESP32 → Mac OCR → Result displayed.

## Test Scenarios

| ID | Scenario | Expected Result | Status |
|---|---|---|---|
| IT-01 | Upload clear plate photo, server running | Plate text displayed in < 30 s | ✅ Verified |
| IT-02 | Upload photo, server NOT running | "Could not read plate" shown, LED blinks fast | ✅ Verified |
| IT-03 | Image > 45 KB after compression | JS retries with lower quality until fit | ✅ Verified |
| IT-04 | ESP32 boots without Mac connected | Default IP 192.168.4.2, waits for `/register` | ✅ Verified |
| IT-05 | Mac starts OCR server, auto-registers | "Registered with ESP32" log, IP updated | ✅ Verified |
| IT-06 | Real plate with state name text | Regex extracts plate only, state name discarded | ✅ Verified |
| IT-07 | Plate with zeros (e.g. ABC-000-D) | `fix_plate` corrects N→0 in digit positions | ✅ Verified |

## How to run a full integration test

1. Connect Mac to **LicensePlateReader** WiFi.
2. Start OCR server: `python tools/scripts/ocr_server.py`
3. Open `http://192.168.4.1:5000` on iPhone.
4. Tap **Analyze**, select/take plate photo.
5. Verify result on screen and in Mac terminal log.

## Expected terminal output (Mac)

```
[HH:MM:SS] INFO: Received 14823 bytes from ESP32
[HH:MM:SS] INFO: Plate region: (12,45) 310x80
[HH:MM:SS] INFO: Main char row: y=10..68 (from 80px total)
[HH:MM:SS] INFO: Best: 'ABC000D' conf=0.81
[HH:MM:SS] INFO: OCR result: 'ABC000D' (confidence: 0.81) in 8.42s
```
