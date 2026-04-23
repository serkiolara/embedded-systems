#include "ResultTransmitter.h"

ResultTransmitter::ResultTransmitter() {}

void ResultTransmitter::sendResult(WebServer& server, const String& plate, float confidence) {
    StaticJsonDocument<256> doc;
    doc["status"] = "success";
    doc["plate"] = plate;
    doc["confidence"] = confidence;
    doc["timestamp"] = millis();

    String response;
    serializeJson(doc, response);

    server.send(200, "application/json", response);
    Serial.print("[Result] Sent: ");
    Serial.println(response);
}

void ResultTransmitter::sendError(WebServer& server, const String& message, int code) {
    StaticJsonDocument<256> doc;
    doc["status"] = "error";
    doc["message"] = message;
    doc["code"] = code;

    String response;
    serializeJson(doc, response);

    server.send(code, "application/json", response);
    Serial.print("[Result] Error: ");
    Serial.println(response);
}
