#pragma once

#include <Arduino.h>
#include <Stream.h>
#include <cstdint>

// Read-only class
class MemoryStream : public Stream {
public:
    MemoryStream(const uint8_t* buffer, size_t size);

    int available() override;
    int peek() override;
    int read() override;
    void flush() override;
    size_t write(uint8_t) override;
    using Print::write;
    void reset();
    bool seek(size_t pos);

private:
    const uint8_t* _buffer;
    size_t _size;
    size_t _position;
};
