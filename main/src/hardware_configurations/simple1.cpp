#include "simple1.h"

namespace hardware {

	HardwareConfiguration simple1 = {
		.name = "simple1",
		.fanPin = 21,
		.resetPin = 5,
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

		.outputs = {20, 8, 9, 10},

		.requires_shorted = {6, 7, 21}
	};

}
