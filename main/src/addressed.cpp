#include "addressed.h"
#define INTR_CPU_ID_AUTO ESP_INTR_CPU_AFFINITY_AUTO
#include <cmath>
#include <vector>

#include "lib/ESP32DMASPIMaster/ESP32DMASPIMaster.h"
#include "constrain.h"
#include "common_types.h"
#include "lib/fixedpoint/fixedpoint.h"

namespace addressed {

    ESP32DMASPI::Master master;

    bool initialized = false;
    std::uint8_t* spi_buf = master.allocDMABuffer(MAX_ADDRESSED_BUF_SIZE);
    std::uint8_t color_buf[MAX_COLOR_BUF_SIZE];
    unsigned spi_buf_size;
    unsigned long long int expected_end_transmission_time = 0; // micros

    unsigned address_count = 300*3;
    unsigned period_ns = 50;

    // nanoseconds
    unsigned T0H = 350;
    unsigned T1H = 700;
    unsigned T0L = 800;
    unsigned T1L = 600;
    unsigned reset = 50000;

    // max 64 bits per bit / 3.2ns at 20MHz
    std::uint64_t one_mask;
    std::uint32_t one_length;
    std::uint64_t zero_mask;
    std::uint32_t zero_length;

    struct SignalMasks {
        std::uint64_t one;
        std::uint64_t one_length;
        std::uint64_t zero;
        std::uint64_t zero_length;
    };


    struct LedSegment {
        unsigned count;
        fixed32_c offset;
        fixed32_c scale; 
        signed char direction; //-1 or 1
        char channel_count;
        std::array<char, 5> channel_mapping;
    };


    using fixedv5 = std::array<fixed32_c, 5>;


    struct ColorChange {
        fixedv5 value; //red, green, blue, white-warm, white-cold
        fixed32_c speed;
        std::uint64_t time; // milliseconds
        fixed32_c distance; // dt * speed
        bool used;
    };


    void fillSegment(fixedv5 current_color, const std::vector<ColorChange*> changes, const LedSegment& segment, std::uint8_t* buf) {
        unsigned changes_size = changes.size();
        fixed32_c distance = segment.count * segment.scale + segment.offset;
        std::uint8_t* buf_end = buf + segment.count;
        std::uint8_t* buf_it = segment.direction >= 0 ? buf : (buf_end - segment.channel_count);
        int buf_inc = segment.direction >= 0 ? segment.channel_count : -segment.channel_count;
        for (unsigned i=changes_size-1;~i && buf_it >= buf && buf_it < buf_end;i--) {
            ColorChange* current_change = changes[i];
            fixed32_c step = 1 / (distance - current_change->distance);
            fixedv5 color_step;
            for (int c=0;c<5;c++)
                color_step[c] = (current_change->value[c] - current_color[c]) * step;
            while (distance > current_change->distance && buf_it >= buf && buf_it < buf_end) {
                distance -= step;
                for (int c=0;c<5;c++)
                    current_color[c] -= color_step[c];
                for (int c=segment.channel_count;~c;c--)
                    buf_it[c] = (std::uint8_t)constrain<int>(current_color[segment.channel_mapping[c]], 0, 255);
                buf_it += buf_inc;
            }
        }
        char fill_color[5];
        for (int c=segment.channel_count;~c;c--)
            fill_color[c] = (std::uint8_t)constrain<int>(current_color[segment.channel_mapping[c]], 0, 255);
        while (buf_it >= buf && buf_it < buf_end) {
            for (int c=segment.channel_count;~c;c--)
                buf_it[c] = fill_color[c];
            buf_it += buf_inc;
        }
    }


    std::uint64_t createMask(unsigned freq, unsigned th, unsigned tl, std::uint32_t* length_ptr) {
        float freqGHz = (float)freq *1e-9;
        unsigned length = round((tl + th) * freqGHz);
        unsigned th_length = round(th * freqGHz);
        if (length_ptr) *length_ptr = length;
        return ((std::uint64_t)1 << th_length) - 1;
    }


    void spiBeforeSettings() {
        if (initialized) {
            while (!transmissionFinished()) delay(1);
            master.end();
        }
        initialized = true;
    }


    void updateConfiguraion() {
        spiBeforeSettings();
        unsigned freq = 20000000;
        period_ns = 1000000000llu / freq;
        master.setDataMode(SPI_MODE0);
        master.setFrequency(freq); 
        master.setMaxTransferSize(MAX_ADDRESSED_BUF_SIZE);
        master.setQueueSize(1);
        master.setDeviceFlags(SPI_DEVICE_TXBIT_LSBFIRST);
        Serial.print("Initialization SPI result: ");
        Serial.println(master.begin(FSPI, -1, -1, 10, -1)); //uint8_t spi_bus, int sck, int miso, int mosi, int ss

        one_mask = createMask(freq, T1H, T1L, &one_length);
        zero_mask = createMask(freq, T0H, T0L, &zero_length);

        Serial.printf("One mask length: %ld\n", one_length);
        for (int i=0;i<one_length;i++)
            Serial.print((one_mask & (1 << i))?1:0);
        Serial.println();
        Serial.printf("Zero mask length: %lu\n", zero_length);
        for (int i=0;i<zero_length;i++)
            Serial.print((zero_mask & (1 << i))?1:0);
        Serial.println();
    }


    bool transmissionFinished() {
        return micros() > expected_end_transmission_time && master.numTransactionsInFlight() == 0;
    }


    void queueSpiBuffer() {
        if (!transmissionFinished()) return;
        Serial.println("Here - queue spi buffer");
        Serial.print("Scheduling task: ");
        Serial.print(master.queue(spi_buf, NULL, spi_buf_size));
        Serial.println(master.trigger());
        expected_end_transmission_time = micros() + ((spi_buf_size * period_ns * 8 + reset) / 1000) + 1;
    }


    template<typename MASK_T>
    unsigned putByteToBuffer(unsigned bc, std::uint8_t* buf, std::uint8_t byte, const SignalMasks& masks) {
        for (unsigned i=7;~i;i--) {
            unsigned bc_bit = bc & 7;
            bool bit = (byte & (1 << i));
            MASK_T or_mask = (bit ? masks.one : masks.zero) << bc_bit;
            MASK_T and_mask = (((MASK_T)1 << bc_bit) - 1);
            MASK_T* buf_ptr = (MASK_T*)(buf + (bc >> 3)); 
            *buf_ptr = ((*buf_ptr) & and_mask) | or_mask;
            bc += bit ? masks.one_length : masks.zero_length;
        }
        return bc;
    }


    template<typename MASK_T>
    unsigned fillSpiBufferTpl(const SignalMasks& masks) {
        unsigned bc = 0;
        std::uint8_t* loop_end = color_buf + address_count;
        for (std::uint8_t* color=color_buf;color<loop_end;color++)
            bc = putByteToBuffer<MASK_T>(bc, spi_buf, *color, masks);
        return bc;
    }


    void finishSpiBuffer(unsigned bc) {
        spi_buf_size = (bc + 7) >> 3;
        while (spi_buf_size & 3) 
            spi_buf[spi_buf_size++] = 0;
    }


    void fillSpiBuffer() {
        SignalMasks masks = (SignalMasks){one_mask, one_length, zero_mask, zero_length};
        unsigned bc = (masks.one_length <= 25 && masks.zero_length <= 25) ?
            fillSpiBufferTpl<std::uint32_t>(masks) : fillSpiBufferTpl<std::uint64_t>(masks);
        finishSpiBuffer(bc);
    }


    String byteToStr(std::uint8_t x) {
        char buf[9];
        buf[8] = 0;
        for (int i=0;i<8;i++)
            buf[i] = (x & (1 << i)) ? '1' : '0';
        return String(buf);
    }


    unsigned test_i = 0;
    void test() {
        //test
        address_count = 3;
        color_buf[0] = (test_i & 3) << 6; //g
        color_buf[1] = (test_i & 12) << 4; //r
        color_buf[2] = (test_i & 48) << 2; //b
        Serial.printf("%d, %d, %d\n", color_buf[0], color_buf[1], color_buf[2]);
        test_i++;
        fillSpiBuffer();
        for (int i=0;i<spi_buf_size;i++)
            Serial.print(byteToStr(spi_buf[i]));
        Serial.println();
        queueSpiBuffer();        
        while (!transmissionFinished()) delay(1);
        Serial.println("Transmission finished");
    }


}
