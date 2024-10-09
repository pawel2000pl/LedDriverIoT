#include "ledc_driver.h"
#include <Arduino.h>
#include <driver/ledc.h>
#include "constrain.h"


unsigned current_pwm_frequency = 32000;


void initLedC(void) {
	ledc_timer_config_t ledc_timer = {
		.speed_mode       = LEDC_MODE,
		.duty_resolution  = LEDC_DUTY_RES,
		.timer_num        = LEDC_TIMER,
		.freq_hz          = current_pwm_frequency,  
		.clk_cfg          = LEDC_AUTO_CLK
	};
	// screw errors, it works fine
	ledc_timer_config(&ledc_timer);
}


void checkNewFrequency(unsigned freq) {
	if (freq >= 1000 && freq <= 32000 && freq != current_pwm_frequency) {
		current_pwm_frequency = freq;
		ledc_set_freq(LEDC_MODE, LEDC_TIMER, current_pwm_frequency);
	}
}


struct ChannelCache {
		bool initialized = false;
		uint32_t duty = 0;
		int hpoint = 9;
		bool invert = false;
};


float addGateLoadingTime(float value, float loadingTime) {
		if (value == 0)
				return 0;
		float offset = loadingTime * float(current_pwm_frequency) * 1e-6;
		return value / (1.f - offset) + offset;
}


ChannelCache cache[4];

void setLedC(int gpio, unsigned channel, float value, float phase, bool invert) {
	uint32_t duty = constrain<uint32_t>(round(value * (LEDC_PERIOD-1)), 0, LEDC_PERIOD-1);
	int hpoint = constrain<int>(round(phase * (LEDC_PERIOD - 1)), 0, LEDC_PERIOD-1);
	if (cache[channel].initialized && cache[channel].duty == duty && cache[channel].hpoint == hpoint && cache[channel].invert == invert)
		return;
	cache[channel].hpoint = hpoint;
	cache[channel].duty = duty;
	cache[channel].invert = invert;
	cache[channel].initialized = true;
	ledc_channel_config_t ledc_channel = {
		.gpio_num       = gpio,
		.speed_mode     = LEDC_MODE,
		.channel        = (ledc_channel_t)channel,
		.intr_type      = LEDC_INTR_DISABLE,
		.timer_sel      = LEDC_TIMER,
		.duty           = duty,
		.hpoint         = hpoint,
		.flags          = { .output_invert = invert ? 1 : 0 }
	};
	ledc_channel_config(&ledc_channel);
}

