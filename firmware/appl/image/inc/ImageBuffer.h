#ifndef IMAGE_BUFFER_H
#define IMAGE_BUFFER_H

#include <Arduino.h>
#include <WiFiClient.h>

#define MAX_IMAGE_SIZE (60 * 1024)  // 60 KB

enum ImageFormat {
    IMG_JPEG,
    IMG_PNG,
    IMG_UNKNOWN
};

class ImageBuffer {
private:
    static uint8_t _data[MAX_IMAGE_SIZE];
    uint32_t _size;
    uint32_t _maxSize;
    ImageFormat _format;
    bool _ready;

    ImageFormat detectFormat();

public:
    ImageBuffer();
    bool receiveImage(WiFiClient& client, uint32_t sizeBytes);
    void setSize(uint32_t size);
    bool isReady() const;
    uint8_t* getData();
    uint32_t getSize() const;
    void clear();
    ImageFormat getFormat() const;
};

#endif
