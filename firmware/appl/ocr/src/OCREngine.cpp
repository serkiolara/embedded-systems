#include "OCREngine.h"

OCREngine::OCREngine()
    : _serverPort(5001), _connected(false), _timeout(30000) {}

void OCREngine::setServer(const String& ip, uint16_t port) {
    _serverIP = ip;
    _serverPort = port;
}

bool OCREngine::connectToCompanion() {
    Serial.print("[OCR] Connecting to Python companion at ");
    Serial.print(_serverIP);
    Serial.print(":");
    Serial.println(_serverPort);

    _connected = _client.connect(_serverIP.c_str(), _serverPort);

    if (_connected) {
        Serial.println("[OCR] Connected to companion");
    } else {
        Serial.println("[OCR] WARNING: Could not connect to companion");
    }

    return _connected;
}

OCRResult OCREngine::processImage(uint8_t* imageData, uint32_t size) {
    OCRResult result = {"", 0.0, false};

    // Try to connect to the Python companion
    if (!_client.connect(_serverIP.c_str(), _serverPort)) {
        Serial.println("[OCR] Companion not available, using mock response");
        // Return mock result when companion is not available
        result.plate = "MOCK-1234";
        result.confidence = 0.0;
        result.valid = false;
        return result;
    }

    // Send HTTP POST request with image data
    _client.println("POST /ocr HTTP/1.1");
    _client.print("Host: ");
    _client.print(_serverIP);
    _client.print(":");
    _client.println(_serverPort);
    _client.println("Content-Type: application/octet-stream");
    _client.print("Content-Length: ");
    _client.println(size);
    _client.println("Connection: close");
    _client.println();

    // Send image data in chunks
    uint32_t sent = 0;
    while (sent < size) {
        uint32_t chunkSize = min((uint32_t)1024, size - sent);
        _client.write(imageData + sent, chunkSize);
        sent += chunkSize;
    }

    // Wait for response
    unsigned long startTime = millis();
    while (_client.connected() && !_client.available()) {
        if (millis() - startTime > _timeout) {
            Serial.println("[OCR] Response timeout");
            _client.stop();
            return result;
        }
        delay(10);
    }

    // Skip HTTP headers
    while (_client.available()) {
        String line = _client.readStringUntil('\n');
        if (line == "\r") break;
    }

    // Read JSON body
    String body = _client.readString();
    _client.stop();

    Serial.print("[OCR] Response: ");
    Serial.println(body);

    // Parse JSON
    StaticJsonDocument<256> doc;
    DeserializationError error = deserializeJson(doc, body);

    if (!error) {
        result.plate = doc["plate"].as<String>();
        result.confidence = doc["confidence"].as<float>();
        result.valid = (result.confidence > 0.5);
        Serial.print("[OCR] Plate: ");
        Serial.print(result.plate);
        Serial.print(" (confidence: ");
        Serial.print(result.confidence);
        Serial.println(")");
    } else {
        Serial.print("[OCR] JSON parse error: ");
        Serial.println(error.c_str());
    }

    return result;
}

void OCREngine::disconnect() {
    if (_client.connected()) {
        _client.stop();
    }
    _connected = false;
}

bool OCREngine::isConnected() const {
    return _connected;
}
