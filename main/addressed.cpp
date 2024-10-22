#include "addressed.h"
#include <cmath>

namespace addressed {

    using uint64 = unsigned long long int;
    using uint32 = unsigned;
    using uint16 = unsigned short;
    using uint8 = unsigned char;

    uint8 spi_buf[MAX_ADDRESSED_BUF_SIZE];
    uint8 color_buf[MAX_COLOR_BUF_SIZE];
    unsigned spi_buf_size;

    unsigned address_count = 300*3;
    unsigned cpu_ticks_per_bit = 8;

    // nanoseconds
    unsigned T0H = 350;
    unsigned T1H = 700;
    unsigned T0L = 800;
    unsigned T1L = 600;
    unsigned reset = 50000;

    // max 64 bits per bit / 3.2ns at 20MHz
    uint64 one_mask;
    uint32 one_length;
    uint64 zero_mask;
    uint32 zero_length;


    uint64 createMask(unsigned ticks, unsigned tl, unsigned th, unsigned* length_ptr) {
        float freq = (float)CPU_FREQ / ((float)ticks * 1e9f);
        unsigned length = round((tl + th) * freq);
        unsigned th_length = round(th * freq);
        uint32 base_or = ((uint32)1 << (th_length+1)) - 1;
        if (length_ptr) *length_ptr = length;
        return base_or;
    }


    template<typename MASK_T>
    void fillSpiBufferTpl() {
        unsigned bc = 0;
        uint8* loop_end = color_buf + address_count;
        for (uint8* color=color_buf;color<loop_end;color++) {
            for (unsigned i=7;~i;i--) {
                int bc_bit = bc & 7;
                bool bit = ((*color) & (1 << i));
                MASK_T or_mask = bit ? one_mask : zero_mask;
                MASK_T and_mask = (((MASK_T)1 << bc_bit) - 1);
                MASK_T* spi_ptr = (MASK_T*)(spi_buf + (bc >> 3)); 
                *spi_ptr = ((*spi_ptr) & and_mask) | (or_mask << bc_bit);
                bc += bit ? one_length : zero_length;
            }
        }
        spi_buf_size = bc >> 3;
    }


    void fillSpiBuffer() {
        if (one_length <= 24 && zero_length <= 24)
            fillSpiBufferTpl<uint32>();
        else
            fillSpiBufferTpl<uint64>();
    }



}
