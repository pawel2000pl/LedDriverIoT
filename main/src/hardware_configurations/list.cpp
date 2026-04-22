#include "../hardware_configuration.h"
#include "../inplace_vector.h"


namespace hardware {

    extern HardwareConfiguration multiplexer1;
    extern HardwareConfiguration multiplexer2;
    extern HardwareConfiguration multiplexer3;
    extern HardwareConfiguration simple1;
    extern HardwareConfiguration simple2;
    extern HardwareConfiguration unconnected;


    inplace_vector<HardwareConfiguration*, 16> configurations = {
        &simple1,
        &simple2,
        &multiplexer1,
        &multiplexer2,
        &multiplexer3,
        &unconnected,
    };

}
