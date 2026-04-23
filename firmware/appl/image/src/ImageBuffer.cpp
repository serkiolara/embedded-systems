#include "ImageBuffer.h"

uint8_t ImageBuffer::_data[MAX_IMAGE_SIZE];

ImageBuffer::ImageBuffer()
    : _size(0), _maxSize(MAX_IMAGE_SIZE), _format(IMG_UNKNOWN), _ready(false) {}

ImageFormat ImageBuffer::detectFormat() {
    if (_size < 4) return IMG_UNKNOWN;

    // JPEG magic: FF D8 FF
    if (_data[0] == 0xFF && _data[1] == 0xD8 && _data[2] == 0xFF) {
        return IMG_JPEG;
    }
    // PNG magic: 89 50 4E 47
    if (_data[0] == 0x89 && _data[1] == 0x50 && _data[2] == 0x4E && _data[3] == 0x47) {
        return IMG_PNG;
    }
    return IMG_UNKNOWN;
}

bool ImageBuffer::receiveImage(WiFiClient& client, uint32_t sizeBytes) {
    if (sizeBytes > _maxSize) {
        Serial.println("[Image] ERROR: Image exceeds buffer size");
        return false;
    }

    clear();

    unsigned long startTime = millis();
    const unsigned long timeout = 5000;  // 5 second timeout

    while (_size < sizeBytes && client.connected()) {
        if (millis() - startTime > timeout) {
            Serial.println("[Image] ERROR: Receive timeout");
            clear();
            return false;
        }

        if (client.available()) {
            int bytesRead = client.readBytes(
                _data + _size,
                min((size_t)(sizeBytes - _size), (size_t)1024)
            );
            _size += bytesRead;
            startTime = millis();  // Reset timeout on data received
        }
    }

    if (_size == sizeBytes) {
        _format = detectFormat();
        _ready = true;
        Serial.print("[Image] Received ");
        Serial.print(_size);
        Serial.print(" bytes (");
        Serial.print(_format == IMG_JPEG ? "JPEG" : _format == IMG_PNG ? "PNG" : "UNKNOWN");
        Serial.println(")");
        return true;
    }

    Serial.println("[Image] ERROR: Incomplete transfer");
    clear();
    return false;
}

bool ImageBuffer::isReady() const {
    return _ready;
}

void ImageBuffer::setSize(uint32_t size) {
    _size = size;
    if (_size > 0) {
        _format = detectFormat();
        _ready = true;
    }
}

uint8_t* ImageBuffer::getData() {
    return _data;
}

uint32_t ImageBuffer::getSize() const {
    return _size;
}

void ImageBuffer::clear() {
    _size = 0;
    _format = IMG_UNKNOWN;
    _ready = false;
}

ImageFormat ImageBuffer::getFormat() const {
    return _format;
}
