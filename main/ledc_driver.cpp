#include "ledc_driver.h"
#include <Arduino.h>
#include <driver/ledc.h>
#include "constrain.h"

const unsigned PWM_FREQUENCES[] = {
  1000,
  1600,
  2000,
  2400,
  4000,
  6000,
  8000,
  12000,
  16000,
  24000,
  32000,
  0
};

unsigned length(const unsigned* ptr) {
  unsigned i = 0;
  while (*(ptr++)) i++;
  return i;
}

const unsigned PWM_FREQUENCES_COUNT = length(PWM_FREQUENCES);
unsigned current_pwm_frequency = PWM_FREQUENCES_COUNT - 1;

void initLedC(void) {
  ledc_timer_config_t ledc_timer = {
    .speed_mode       = LEDC_MODE,
    .duty_resolution  = LEDC_DUTY_RES,
    .timer_num        = LEDC_TIMER,
    .freq_hz          = PWM_FREQUENCES[current_pwm_frequency],  
    .clk_cfg          = LEDC_AUTO_CLK
  };
  // screw errors, it works fine
  ledc_timer_config(&ledc_timer);
}

void checkNewFrequency(unsigned number) {
  if (number >= 0 && number < PWM_FREQUENCES_COUNT && number != current_pwm_frequency) {
    current_pwm_frequency = number;
    initLedC();
  }
}

struct ChannelCache {
    bool initialized = false;
    uint32_t duty = 0;
    int hpoint = 9;
};

float addGateLoadingTime(float value, float loadingTime) {
    if (value == 0)
        return 0;
    float offset = loadingTime * float(PWM_FREQUENCES[current_pwm_frequency]) * 1e-6;
    return value / (1.f - offset) + offset;
}

ChannelCache cache[4];

void setLedC(int gpio, unsigned channel, float value, bool invert) {
  value = constrain<float>(value, 0, 1);
  uint32_t duty = constrain<uint32_t>(round(value * (LEDC_PERIOD-1)), 0, LEDC_PERIOD-1);
  int hpoint = constrain<int>(round((1.f - value) * (LEDC_PERIOD / 2 - 1)), 0, LEDC_PERIOD / 2 - 1);
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

