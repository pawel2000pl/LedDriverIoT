#include "../hardware_configuration.h"


namespace hardware {

	HardwareConfiguration multiplexer1 = {
		.name = "multiplexer1",
		.fanPin = 4,
		.resetPin = 5,
		.potentiometers = {
			InputHardwareAction{
				.enabled = true,
				.read_pin = 2,
				.hz_pins = {},
				.low_pins = {6, 7, 21},
				.high_pins = {},
			},
			InputHardwareAction{
				.enabled = true,
				.read_pin = 2,
				.hz_pins = {},
				.low_pins = {7, 21},
				.high_pins = {6},
			},
			InputHardwareAction{
				.enabled = true,
				.read_pin = 2,
				.hz_pins = {},
				.low_pins = {6, 21},
				.high_pins = {7},
			},
			InputHardwareAction{
				.enabled = true,
				.read_pin = 2,
				.hz_pins = {},
				.low_pins = {21},
				.high_pins = {6, 7},
			},
		},

		.thermistors = {
			InputHardwareAction{
				.enabled = true,
				.read_pin = 2,
				.hz_pins = {},
				.low_pins = {6, 7},
				.high_pins = {21},
			},
			InputHardwareAction{
				.enabled = true,
				.read_pin = 2,
				.hz_pins = {},
				.low_pins = {7},
				.high_pins = {6, 21},
			},
			InputHardwareAction{
				.enabled = true,
				.read_pin = 2,
				.hz_pins = {},
				.low_pins = {6},
				.high_pins = {7, 21},
			},
			InputHardwareAction{
				.enabled = true,
				.read_pin = 2,
				.hz_pins = {},
				.low_pins = {},
				.high_pins = {6, 7, 21},
			},
		},
		.outputs = {20, 8, 9, 10},

		.requires_potentiometers = {2},
		.requires_hz = {3, 6, 7, 21},
		.requires_not_shorted = {6, 7, 10, 21}
	};

}
