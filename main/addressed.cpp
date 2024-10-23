#include "addressed.h"
#include <ESP32DMASPIMaster.h>
#include <cmath>

namespace addressed {

    using uint64 = unsigned long long int;
    using uint32 = unsigned;
    using uint16 = unsigned short;
    using uint8 = unsigned char;

    ESP32DMASPI::Master master;

    bool initialized = false;
    uint8* spi_buf = master.allocDMABuffer(MAX_ADDRESSED_BUF_SIZE);
    uint8 color_buf[MAX_COLOR_BUF_SIZE];
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
    uint64 one_mask;
    uint32 one_length;
    uint64 zero_mask;
    uint32 zero_length;


    uint64 createMask(unsigned freq, unsigned th, unsigned tl, unsigned* length_ptr) {
        float freqGHz = (float)freq *1e-9;
        unsigned length = round((tl + th) * freqGHz);
        unsigned th_length = round(th * freqGHz);
        if (length_ptr) *length_ptr = length;
        return ((uint64)1 << th_length) - 1;
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

        Serial.printf("One mask length: %d\n", one_length);
        for (int i=0;i<one_length;i++)
            Serial.print((one_mask & (1 << i))?1:0);
        Serial.println();
        Serial.printf("Zero mask length: %d\n", zero_length);
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
    void fillSpiBufferTpl() {
        unsigned bc = 0;
        uint8* loop_end = color_buf + address_count;
        for (uint8* color=color_buf;color<loop_end;color++) {
            for (unsigned i=7;~i;i--) {
                unsigned bc_bit = bc & 7;
                bool bit = ((*color) & (1 << i));
                MASK_T or_mask = (bit ? one_mask : zero_mask) << bc_bit;
                MASK_T and_mask = (((MASK_T)1 << bc_bit) - 1);
                MASK_T* spi_ptr = (MASK_T*)(spi_buf + (bc >> 3)); 
                *spi_ptr = ((*spi_ptr) & and_mask) | or_mask;
                bc += bit ? one_length : zero_length;
            }
        }
        spi_buf_size = bc >> 3;
        do {
            spi_buf[spi_buf_size++] = 0;
        } while (spi_buf_size & 3);
    }


    void fillSpiBuffer() {
        if (one_length <= 25 && zero_length <= 25)
            fillSpiBufferTpl<uint32>();
        else
            fillSpiBufferTpl<uint64>();
    }


    String byteToStr(uint8 x) {
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
