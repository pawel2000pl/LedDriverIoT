#include "hal/ledc_types.h"
#include <driver/ledc.h>


#define LEDC_TIMER              LEDC_TIMER_0
#define LEDC_MODE               LEDC_LOW_SPEED_MODE
#define LEDC_CHANNEL            LEDC_CHANNEL_0
#define LEDC_DUTY_RES           LEDC_TIMER_11_BIT
#define LEDC_FREQUENCY          (32000)


void initLedC(void) {
  ledc_timer_config_t ledc_timer = {
    .speed_mode       = LEDC_MODE,
    .duty_resolution  = LEDC_DUTY_RES,
    .timer_num        = LEDC_TIMER,
    .freq_hz          = LEDC_FREQUENCY,  
    .clk_cfg          = LEDC_AUTO_CLK
  };
  // srew errors, it works fine
  ledc_timer_config(&ledc_timer);
}


void setLedC(unsigned gpio, unsigned channel, float value) {
  ledc_channel_config_t ledc_channel = {
    .gpio_num       = gpio,
    .speed_mode     = LEDC_MODE,
    .channel        = (ledc_channel_t)channel,
    .intr_type      = LEDC_INTR_DISABLE,
    .timer_sel      = LEDC_TIMER,
    .duty           = floor(value * 2047),
    .hpoint         = floor((1 - value) * 1023)
  };
  ledc_channel_config(&ledc_channel);
}

