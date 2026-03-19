/**
 * @file main.cpp
 * @brief License Plate Reader - ESP32 Main Entry Point
 * @author Andrea Venegas, Sergio Lara
 * @date 2026-03-18
 * @version 1.0
 */

#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>

// ============================================================================
// CONFIGURATION
// ============================================================================

#define LED_PIN 2
#define WIFI_SSID "LicensePlateReader"
#define WIFI_PASSWORD ""
#define SERVER_PORT 5000
#define MAX_IMAGE_SIZE (400 * 1024)  // 400 KB

// ============================================================================
// GLOBAL OBJECTS
// ============================================================================

WebServer server(SERVER_PORT);
uint8_t imageBuffer[MAX_IMAGE_SIZE];
uint32_t imageSize = 0;
bool imageReady = false;

// ============================================================================
// LED CONTROLLER (Simple implementation)
// ============================================================================

enum LEDState {
    IDLE,
    PROCESSING,
    DONE,
    ERROR
};

class LEDController {
private:
    uint8_t pin;
    LEDState state;
    unsigned long lastToggle;
    bool ledOn;
    uint32_t blinkInterval;

public:
    LEDController(uint8_t pin) : pin(pin), state(IDLE), lastToggle(0), ledOn(false), blinkInterval(500) {
        pinMode(pin, OUTPUT);
        digitalWrite(pin, LOW);
    }

    void setState(LEDState newState) {
        state = newState;
        lastToggle = millis();
        ledOn = false;
        digitalWrite(pin, LOW);

        switch (state) {
            case IDLE:
                digitalWrite(pin, LOW);
                break;
            case PROCESSING:
                digitalWrite(pin, HIGH);
                break;
            case DONE:
                blinkInterval = 500;  // 1 Hz
                break;
            case ERROR:
                blinkInterval = 100;  // 5 Hz
                break;
        }
    }

    void update() {
        if (state == IDLE || state == PROCESSING) {
            return;  // No blinking needed
        }

        unsigned long now = millis();
        if (now - lastToggle >= blinkInterval) {
            ledOn = !ledOn;
            digitalWrite(pin, ledOn ? HIGH : LOW);
            lastToggle = now;
        }
    }
};

LEDController ledController(LED_PIN);

// ============================================================================
// WIFI MANAGER
// ============================================================================

void initWiFi() {
    Serial.println("[WiFi] Initializing WiFi AP mode...");

    WiFi.mode(WIFI_AP);
    WiFi.softAP(WIFI_SSID, WIFI_PASSWORD);

    IPAddress ip = WiFi.softAPIP();
    Serial.print("[WiFi] AP started: ");
    Serial.println(WIFI_SSID);
    Serial.print("[WiFi] IP address: ");
    Serial.println(ip);
    Serial.print("[WiFi] Listen on port: ");
    Serial.println(SERVER_PORT);

    ledController.setState(IDLE);
}

// ============================================================================
// HTTP HANDLERS
// ============================================================================

void handleImageUpload() {
    Serial.println("[Server] Received image upload request");
    ledController.setState(PROCESSING);

    if (server.method() != HTTP_POST) {
        server.send(405, "application/json", "{\"error\": \"Method not allowed\"}");
        return;
    }

    // Receive JPEG data
    uint32_t contentLength = server.contentLength();
    if (contentLength > MAX_IMAGE_SIZE) {
        Serial.println("[Server] Image too large!");
        server.send(413, "application/json", "{\"error\": \"Image too large\"}");
        ledController.setState(ERROR);
        delay(2000);
        ledController.setState(IDLE);
        return;
    }

    imageSize = 0;
    WiFiClient client = server.client();

    // Read image data from POST body
    while (client.available() && imageSize < contentLength) {
        int bytesRead = client.readBytes(imageBuffer + imageSize,
                                         min((size_t)(contentLength - imageSize),
                                             (size_t)1024));
        imageSize += bytesRead;
    }

    Serial.print("[Image] Received ");
    Serial.print(imageSize);
    Serial.println(" bytes");

    imageReady = true;

    // TODO: Call image processing pipeline here
    // TODO: Call OCR engine
    // TODO: Get result

    // Mock response for now
    String result = "{\"status\": \"success\", \"plate\": \"ABC1234\", \"confidence\": 0.85}";
    server.send(200, "application/json", result);

    ledController.setState(DONE);
    delay(2000);  // Blink for 2 seconds
    ledController.setState(IDLE);
}

void handleNotFound() {
    server.send(404, "application/json", "{\"error\": \"Not found\"}");
}

// ============================================================================
// SETUP
// ============================================================================

void setup() {
    // Serial initialization
    Serial.begin(115200);
    delay(1000);

    Serial.println("\n\n");
    Serial.println("========================================");
    Serial.println("License Plate Reader v1.0");
    Serial.println("ESP32 Firmware");
    Serial.println("========================================");
    Serial.print("Build: ");
    Serial.println(__TIMESTAMP__);
    Serial.println();

    // LED initialization
    ledController.setState(IDLE);

    // WiFi initialization
    initWiFi();

    // Web server routes
    server.on("/upload", HTTP_POST, handleImageUpload);
    server.onNotFound(handleNotFound);

    // Start server
    server.begin();
    Serial.println("[Server] HTTP server started");

    Serial.println("[Boot] Initialization complete. Waiting for client...");
}

// ============================================================================
// MAIN LOOP
// ============================================================================

void loop() {
    // Handle incoming HTTP requests
    server.handleClient();

    // Update LED state (for blinking)
    ledController.update();

    // Small delay to prevent watchdog issues
    delay(10);
}
