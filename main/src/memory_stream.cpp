#include "memory_stream.h"


MemoryStream::MemoryStream(const uint8_t* buffer, size_t size)
    : _buffer(buffer), _size(size), _position(0) {}

int MemoryStream::available() {
    return (_position < _size) ? (_size - _position) : 0;
}

int MemoryStream::peek() {
    if (_position >= _size) return -1;
    return _buffer[_position];
}

int MemoryStream::read() {
    if (_position >= _size) return -1;
    return _buffer[_position++];
}

void MemoryStream::flush() {
}

size_t MemoryStream::write(uint8_t) {
    return 0;
}

void MemoryStream::reset() {
    _position = 0;
}

bool MemoryStream::seek(size_t pos) {
    if (pos > _size) return false;
    _position = pos;
    return true;
}
