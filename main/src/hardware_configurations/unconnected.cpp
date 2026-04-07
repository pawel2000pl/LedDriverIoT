#include "../hardware_configuration.h"


namespace hardware {

	HardwareConfiguration unconnected = {
		.name = "unconnected",
		.fanPin = 21,
		.resetPin = 9,
		.potentiometers = {},
        .thermistors = {},

		.outputs = {5, 6, 7, 8},

		.requires_hz = {2, 3, 4, 5, 6, 7, 8, 16, 20, 21},
		.requires_not_shorted = {6, 7, 16, 21}
	};

}
