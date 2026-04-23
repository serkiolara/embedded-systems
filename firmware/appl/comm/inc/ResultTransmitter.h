#ifndef RESULT_TRANSMITTER_H
#define RESULT_TRANSMITTER_H

#include <Arduino.h>
#include <WebServer.h>
#include <ArduinoJson.h>

class ResultTransmitter {
public:
    ResultTransmitter();
    void sendResult(WebServer& server, const String& plate, float confidence);
    void sendError(WebServer& server, const String& message, int code = 500);
};

#endif
