#pragma once

#include <Arduino.h>
#include <array>

using ColorChannels = std::array<float, 4>;

// DMA max size is 384kB
#define MAX_ADDRESSED_BUF_SIZE (65536)
#define MAX_COLOR_BUF_SIZE (MAX_ADDRESSED_BUF_SIZE / 8)
#define MAX_ADDRESSED_FREQ (20000000)
#define CPU_FREQ (160000000)

namespace addressed {

    void updateConfiguraion();
    bool transmissionFinished();


    //test
    String byteToStr(unsigned char x);
    void test();
}
