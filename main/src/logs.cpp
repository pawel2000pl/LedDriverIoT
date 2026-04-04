#include "logs.h"

namespace logs {

    char buffer[1024] = {0};
    unsigned bufPos = 0;

    Logger logger;

    bool initSerial() {
        Serial.begin(115200);
        return true;
    }

    bool serialInitialized = initSerial();


    Logger::Logger() {
        for (unsigned i=0;i<buf_size;i++) buf[i] = 32;
        buf[buf_size-1] = 13;
    }


    size_t Logger::write(uint8_t value) {
        buf[buf_pos++ & buf_size_mask] = value;
        if (serialInitialized) Serial.write(value);
        return 1;
    }


    unsigned Logger::get_buf(unsigned char* ptr, unsigned offset, unsigned size) {
        unsigned end = offset + size;
        if (end >= buf_size) end = buf_size;
        if (offset >= end) return 0;
        for (unsigned i=offset;i<end;i++)
            ptr[i-offset] = buf[(buf_pos + i) & buf_size_mask];
        return end - offset;
    }

}


