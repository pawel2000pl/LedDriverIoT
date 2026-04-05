#include "simple1.h"

namespace hardware {

	HardwareConfiguration simple2 = {
		.name = "simple2",
		.fanPin = 21,
		.resetPin = 9,
		.potentiometers = {
			InputHardwareAction{
				.enabled = true,
				.read_pin = 2,
				.hz_pins = {},
				.low_pins = {},
				.high_pins = {},
			},		
			InputHardwareAction{
				.enabled = true,
				.read_pin = 3,
				.hz_pins = {},
				.low_pins = {},
				.high_pins = {},
			},		
			InputHardwareAction{
				.enabled = true,
				.read_pin = 4,
				.hz_pins = {},
				.low_pins = {},
				.high_pins = {},
			},		
        },
        .thermistors = {},

		.outputs = {5, 6, 7, 8},

		.requires_shorted = {10, 21}
	};

}
