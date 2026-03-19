# Software Release Reports (SRR)

**Project:** License Plate Reader with ESP32
**Version:** 1.0 (Planning Phase)
**Date:** 2026-03-18
**Authors:** Andrea Venegas, Sergio Lara

---

## Release Overview

This document tracks completion status of requirements and deliverables for each software release cycle corresponding to the 6 development sprints and final release.

---

## Release R0 — Definition & Architecture (Completed: 2026-02-27)

**Sprint:** 0
**Duration:** Week 1
**Status:** ✅ COMPLETED

### Objectives
- Define project scope, vision, and requirements
- Establish team roles and Scrum process
- Select hardware and software stack

### Deliverables
| Item | Status | Notes |
|---|---|---|
| Project Vision Document | ✅ | Complete specification of problem, stakeholders, features |
| SRS (Software Requirements Spec) | ✅ | FR-01 to FR-08, NFR-01 to NFR-05 defined |
| HLR (High-Level Architecture) | ✅ | 6 architectural blocks identified (as of 2026-03-18) |
| LLR (Low-Level Requirements) | ✅ | GPIO pins, libraries, APIs defined (as of 2026-03-18) |
| Hardware Selection | ✅ | ESP32 + WiFi + LED + Serial |
| Team Agreement & DoR/DoD | ✅ | Definition of Ready and Done criteria established |

### Testing
- Architecture review: PASSED

### Known Issues
- None

---

## Release R1 — WiFi Communication (Target: 2026-03-20)

**Sprint:** 1
**Duration:** Week 2 (Mar 13–20)
**Status:** 🔄 IN PROGRESS

### Objectives (from Product Backlog)
- Establish ESP32 as WiFi AP
- Receive JPEG image from mobile phone over TCP
- Store image in memory

### User Stories
- US-01: WiFi AP mode ✅ (code skeleton created 2026-03-18)
- US-02: TCP image reception ✅ (code skeleton with handler created)

### Deliverables
| Item | Target | Status |
|---|---|---|
| WiFiManager module | 2026-03-19 | 🔄 Complete API, needs testing |
| ImageBuffer module | 2026-03-19 | 🔄 Buffer allocation, needs testing |
| main.cpp with WiFi setup | 2026-03-18 | ✅ Created (platformio.ini also created) |
| Integration test: phone connects + sends image | 2026-03-20 | ⏳ Pending |
| Exit criteria: Image received and stored | 2026-03-20 | ⏳ Pending |

### Acceptance Criteria
- [ ] ESP32 broadcasts SSID "LicensePlateReader"
- [ ] Mobile phone can connect with IP 192.168.4.1
- [ ] POST /upload accepts JPEG file
- [ ] Image stored in memory buffer (max 400 KB)
- [ ] LED indicates state (IDLE, PROCESSING, DONE)

### Testing
- Unit tests: ImageBuffer receive logic
- Integration test: Real phone connection + JPEG send

### Known Issues
- None yet

### Dependencies
- None (hardware available)

---

## Release R2 — Image Processing Pipeline (Target: 2026-03-27)

**Sprint:** 2
**Duration:** Week 3 (Mar 20–27)
**Status:** ⏳ PENDING

### Objectives
- Preprocess image (grayscale, threshold, denoise)
- Detect license plate region
- Crop plate rectangle

### User Stories
- US-03: Grayscale + threshold preprocessing
- US-04: Contour-based plate detection (8 points)

### Deliverables
| Item | Target | Status |
|---|---|---|
| ImageProcessor module (C++/skeletom) | 2026-03-22 | ⏳ |
| OpenCV integration or Python companion setup | 2026-03-24 | ⏳ |
| Preprocessing test suite | 2026-03-25 | ⏳ |
| Plate detection model (TFLite or contours) | 2026-03-26 | ⏳ |
| Exit criteria: Plate region detected on test images | 2026-03-27 | ⏳ |

### Acceptance Criteria
- [ ] Grayscale conversion working
- [ ] Binary threshold produces clear plate outline
- [ ] Contour detection extracts bounding box
- [ ] Plate region isolated in >90% of test images

---

## Release R3 — OCR Integration (Target: 2026-04-03)

**Sprint:** 3
**Duration:** Week 4 (Mar 27 – Apr 3)
**Status:** ⏳ PENDING

### Objectives
- Integrate Tesseract OCR (via Python companion or direct)
- Send plate image to OCR engine
- Return recognized text to mobile phone

### User Stories
- US-05: Tesseract OCR integration (8 points)
- US-06: Result transmission back to phone (3 points)

### Deliverables
| Item | Target | Status |
|---|---|---|
| OCREngine module (TCP to Python server) | 2026-03-29 | ⏳ |
| Python companion script (ocr_server.py) | 2026-03-30 | ⏳ |
| ResultTransmitter module (JSON response) | 2026-03-31 | ⏳ |
| E2E test: Image → Plate string | 2026-04-02 | ⏳ |
| Exit criteria: ≥70% OCR accuracy | 2026-04-03 | ⏳ |

### Acceptance Criteria
- [ ] Plate image sent to Python/Tesseract
- [ ] Recognized text returned as JSON
- [ ] Result sent back to client over WiFi
- [ ] Character accuracy ≥70%

---

## Release R4 — Embedded ML & UX (Target: 2026-04-10)

**Sprint:** 4
**Duration:** Week 5 (Apr 3–10)
**Status:** ⏳ PENDING

### Objectives
- Compile TFLite model for plate detection on ESP32
- Reduce external dependency on Python companion
- Implement LED state machine

### User Stories
- US-07: LED state machine (2 points)
- US-08: TFLite plate detection on ESP32 (13 points)

### Deliverables
| Item | Target | Status |
|---|---|---|
| TFLite model training & quantization | 2026-04-05 | ⏳ |
| TFLite inference integration (ESP32) | 2026-04-07 | ⏳ |
| LEDController finalized with tests | 2026-04-08 | ⏳ |
| Exit criteria: TFLite model running | 2026-04-10 | ⏳ |

### Acceptance Criteria
- [ ] TFLite model loads and runs on ESP32
- [ ] Plate detection runs in <800 ms
- [ ] LED states change smoothly
- [ ] No memory overflow

---

## Release R5 — Optimization & Testing (Target: 2026-04-17)

**Sprint:** 5
**Duration:** Week 6 (Apr 10–17)
**Status:** ⏳ PENDING

### Objectives
- Optimize pipeline to meet latency target (≤3 s)
- Write comprehensive unit and integration tests
- Validate OCR accuracy ≥85%

### User Stories
- US-09: Unit tests for preprocessing & OCR (5 points)
- US-10: Finalize architecture validation (5 points)
- US-11: Latency optimization to ≤3 s (5 points)

### Deliverables
| Item | Target | Status |
|---|---|---|
| Unit test suite (Google Test or SimpleTest) | 2026-04-12 | ⏳ |
| Performance benchmarking | 2026-04-14 | ⏳ |
| OCR accuracy report (≥85% threshold) | 2026-04-16 | ⏳ |
| Exit criteria: All tests passing | 2026-04-17 | ⏳ |

### Acceptance Criteria
- [ ] End-to-end latency ≤3 s
- [ ] All unit tests pass
- [ ] OCR accuracy ≥85% on 20 test plates
- [ ] No crashes during 10 min continuous demo

---

## Release R6 — Final Demo & Release (Target: 2026-04-20)

**Sprint:** 6
**Duration:** Week 7 (Apr 17–20)
**Status:** ⏳ PENDING

### Objectives
- Complete live demonstration with real license plate
- Finalize all documentation
- Merge to main branch for delivery

### User Stories
- US-12: Working end-to-end demo (8 points)
- US-13: Complete documentation (3 points)
- US-14: Optional: Plate read history logging (5 points)

### Deliverables
| Item | Target | Status |
|---|---|---|
| Live demo setup & test runs | 2026-04-18 | ⏳ |
| Final documentation review | 2026-04-19 | ⏳ |
| Code merged to main | 2026-04-20 | ⏳ |
| README.md updated with usage | 2026-04-20 | ⏳ |
| Exit criteria: Professor demo acceptance | 2026-04-20 | ⏳ |

### Acceptance Criteria
- [ ] Live demo with real plate: input → output
- [ ] Documentation complete (SRS, HLR, LLR, Vision, API)
- [ ] All code in main branch
- [ ] No critical warnings or errors
- [ ] Professor sign-off

---

## Summary Timeline

```
Week 1 (Feb 20-27)   → R0 (Definition)          ✅ DONE
Week 2 (Feb 27-Mar6) → R1 (WiFi)               ⏳ IN-PROGRESS
Week 3 (Mar 6-13)    → R2 (Image Processing)   ⏳ PENDING
Week 4 (Mar 13-20)   → R3 (OCR)                ⏳ PENDING
Week 5 (Mar 20-27)   → R4 (ML + UX)            ⏳ PENDING
Week 6 (Mar 27-Apr3) → R5 (Testing)            ⏳ PENDING
Week 7 (Apr 3-10)    → R6 (Final Demo)         ⏳ PENDING
         (Apr 10-20)   Final testing & delivery
         (Apr 20)      DELIVERY DEADLINE ✅
```

---

## Key Metrics (As of 2026-03-18)

| Metric | Target | Current | Status |
|---|---|---|---|
| Code completion | 100% | ~15% | 🔄 |
| Documentation completion | 100% | ~75% | ✅ |
| Test coverage | >80% | 0% | ⏳ |
| Bug count | 0 | 0 | ✅ |

---

## Notes for Team

1. **R0 Documentation:** HLR and LLR completed on 2026-03-18 to close documentation gap.
2. **R1 Status:** main.cpp skeleton + platformio.ini created. Ready for testing phase.
3. **Critical Path:** R1 WiFi → R2 Image Proc → R3 OCR (sequential, no parallelization possible).
4. **Resource:** Python companion server (ocr_server.py) can be developed in parallel with R1–R2.
5. **Timeline Risk:** 33 days remain. Each sprint is 1 week. Buffer: 5 weeks for R1–R5 + 2 weeks for R6.

---

**Document Status:** Living document — updated weekly
**Last Updated:** 2026-03-18
**Next Review:** 2026-03-25 (Sprint 1 review)
