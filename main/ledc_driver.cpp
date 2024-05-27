#include "ledc_driver.h"
#include <Arduino.h>
#include <driver/ledc.h>

void initLedC(void) {
  ledc_timer_config_t ledc_timer = {
    .speed_mode       = LEDC_MODE,
    .duty_resolution  = LEDC_DUTY_RES,
    .timer_num        = LEDC_TIMER,
    .freq_hz          = LEDC_FREQUENCY,  
    .clk_cfg          = LEDC_AUTO_CLK
  };
  // screw errors, it works fine
  ledc_timer_config(&ledc_timer);
}

struct ChannelCache {
    bool initialized = false;
    uint32_t duty = 0;
    int hpoint = 9;
};

float addGateLoadingTime(float value, float loadingTime) {
    if (value == 0)
        return 0;
    float offset = loadingTime * float(LEDC_FREQUENCY) * 1e-6;
    return value / (1.f - offset) + offset;
}

ChannelCache cache[4];

void setLedC(int gpio, unsigned channel, float value, bool invert) {
  value = constrain(value, 0, 1);
  uint32_t duty = constrain(round(value * (LEDC_PERIOD-1)), 0, LEDC_PERIOD-1);
  int hpoint = constrain(round((1.f - value) * (LEDC_PERIOD / 2 - 1)), 0, LEDC_PERIOD / 2 - 1);
  if (cache[channel].initialized && cache[channel].duty == duty && cache[channel].hpoint == hpoint)
    return;
  cache[channel].hpoint = hpoint;
  cache[channel].duty = duty;
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

