#ifndef OCR_ENGINE_H
#define OCR_ENGINE_H

#include <Arduino.h>
#include <WiFiClient.h>
#include <ArduinoJson.h>

struct OCRResult {
    String plate;
    float confidence;
    bool valid;
};

class OCREngine {
private:
    String _serverIP;
    uint16_t _serverPort;
    WiFiClient _client;
    bool _connected;
    uint32_t _timeout;

public:
    OCREngine();
    void setServer(const String& ip, uint16_t port);
    bool connectToCompanion();
    OCRResult processImage(uint8_t* imageData, uint32_t size);
    void disconnect();
    bool isConnected() const;
};

#endif
