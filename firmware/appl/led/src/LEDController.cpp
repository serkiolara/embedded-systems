#include "LEDController.h"

LEDController::LEDController(uint8_t pin)
    : _pin(pin), _state(LED_IDLE), _lastToggle(0), _ledOn(false), _blinkInterval(500) {}

void LEDController::init() {
    pinMode(_pin, OUTPUT);
    digitalWrite(_pin, LOW);
    _state = LED_IDLE;
}

void LEDController::setState(LEDState newState) {
    _state = newState;
    _lastToggle = millis();
    _ledOn = false;
    digitalWrite(_pin, LOW);

    switch (_state) {
        case LED_IDLE:
            digitalWrite(_pin, LOW);
            break;
        case LED_PROCESSING:
            digitalWrite(_pin, HIGH);
            break;
        case LED_DONE:
            _blinkInterval = 500;  // 1 Hz blink
            break;
        case LED_ERROR:
            _blinkInterval = 100;  // 5 Hz blink
            break;
    }
}

LEDState LEDController::getState() const {
    return _state;
}

void LEDController::update() {
    if (_state == LED_IDLE || _state == LED_PROCESSING) {
        return;
    }

    unsigned long now = millis();
    if (now - _lastToggle >= _blinkInterval) {
        _ledOn = !_ledOn;
        digitalWrite(_pin, _ledOn ? HIGH : LOW);
        _lastToggle = now;
    }
}
