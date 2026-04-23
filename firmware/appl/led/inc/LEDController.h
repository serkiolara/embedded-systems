#ifndef LED_CONTROLLER_H
#define LED_CONTROLLER_H

#include <Arduino.h>

enum LEDState {
    LED_IDLE,
    LED_PROCESSING,
    LED_DONE,
    LED_ERROR
};

class LEDController {
private:
    uint8_t _pin;
    LEDState _state;
    unsigned long _lastToggle;
    bool _ledOn;
    uint32_t _blinkInterval;

public:
    LEDController(uint8_t pin);
    void init();
    void setState(LEDState newState);
    LEDState getState() const;
    void update();
};

#endif
