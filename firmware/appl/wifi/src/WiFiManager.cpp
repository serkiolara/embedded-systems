#include "WiFiManager.h"

WiFiManager::WiFiManager(const char* ssid, const char* password, uint16_t port)
    : _ssid(ssid), _password(password), _port(port), _apActive(false) {}

bool WiFiManager::initSoftAP() {
    Serial.println("[WiFi] Initializing WiFi AP mode...");

    WiFi.mode(WIFI_AP);
    bool result = WiFi.softAP(_ssid, _password, 6);

    if (result) {
        _apActive = true;
        Serial.print("[WiFi] AP started: ");
        Serial.println(_ssid);
        Serial.print("[WiFi] IP address: ");
        Serial.println(WiFi.softAPIP());
        Serial.print("[WiFi] Listen on port: ");
        Serial.println(_port);
    } else {
        Serial.println("[WiFi] ERROR: Failed to start AP!");
    }

    return result;
}

bool WiFiManager::isConnected() {
    return WiFi.softAPgetStationNum() > 0;
}

IPAddress WiFiManager::getLocalIP() {
    return WiFi.softAPIP();
}

uint8_t WiFiManager::getConnectedClients() {
    return WiFi.softAPgetStationNum();
}
