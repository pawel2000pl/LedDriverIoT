#pragma once
#include <hal/ledc_types.h>

#include "common_types.h"

#define LEDC_TIMER              LEDC_TIMER_0
#define LEDC_MODE               LEDC_LOW_SPEED_MODE
#define LEDC_DUTY_RES           LEDC_TIMER_11_BIT
#define LEDC_PERIOD             (1 << (int)LEDC_TIMER_11_BIT)

void initLedC(void);
void checkNewFrequency(unsigned freq);
fixed64 addGateLoadingTime(fixed64 value, fixed64 loadingTime);
void setLedC(int gpio, unsigned channel, fixed64 value, fixed64 phase=0, bool invert=0);

