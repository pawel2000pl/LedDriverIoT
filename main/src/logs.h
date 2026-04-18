#pragma once

#include "Arduino.h"

namespace logs {


    class Logger : public Print {

        public:
            Logger();
            size_t write(uint8_t) override;
            unsigned get_buf(unsigned char* ptr, unsigned offset, unsigned size);
            static const unsigned buf_size = 1024;
        private:
            static const unsigned buf_size_mask = buf_size - 1;
            unsigned char buf[buf_size] = {0};
            unsigned buf_pos = 0;
    };

    void initSerial();
    extern Logger logger;
    extern bool serialInitialized;

}
