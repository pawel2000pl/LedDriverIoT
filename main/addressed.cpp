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
    unsigned frequency = 20000000;

    // nanoseconds
    unsigned period = 1000000000 / frequency;
    unsigned T0H = 350;
    unsigned T1H = 700;
    unsigned T0L = 800;
    unsigned T1L = 600;
    unsigned reset = 50000;

    uint64 one_mask;
    uint32 one_length;
    uint64 zero_mask;
    uint32 zero_length;


    uint64 createMask(unsigned period, unsigned tl, unsigned th, unsigned* length_ptr) {
        unsigned length = (tl + th + (period >> 1)) / period;
        unsigned th_length = (th + (period >> 1)) / period;
        uint64 base_or = ((uint64)1 << (th_length+1)) - 1;
        if (length_ptr) *length_ptr = length;
        return base_or;
    }


    void fillSpiBuffer() {
        unsigned bc = 0;
        uint8* loop_end = color_buf + address_count;
        for (uint8* color=color_buf;color<loop_end;color++) {
            for (unsigned i=7;~i;i--) {
                int bc_bit = bc & 7;
                bool bit = ((*color) & (1 << i));
                uint64 or_mask = bit ? one_mask : zero_mask;
                uint64 and_mask = (((uint64)1 << bc_bit) - 1);
                uint64* spi_ptr = (uint64*)(spi_buf + (bc >> 3)); 
                *spi_ptr = ((*spi_ptr) & and_mask) | (or_mask << bc_bit);
                bc += bit ? one_length : zero_length;
            }
        }
        spi_buf_size = bc >> 3;
    }

}
