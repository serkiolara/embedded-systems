#ifndef WIFI_MANAGER_H
#define WIFI_MANAGER_H

#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>

class WiFiManager {
private:
    const char* _ssid;
    const char* _password;
    uint16_t _port;
    bool _apActive;

public:
    WiFiManager(const char* ssid, const char* password, uint16_t port);
    bool initSoftAP();
    bool isConnected();
    IPAddress getLocalIP();
    uint8_t getConnectedClients();
};

#endif
