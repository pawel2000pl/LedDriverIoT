#include <hal/ledc_types.h>
#include <driver/ledc.h>

#define LEDC_TIMER              LEDC_TIMER_0
#define LEDC_MODE               LEDC_LOW_SPEED_MODE
#define LEDC_DUTY_RES           LEDC_TIMER_11_BIT
#define LEDC_FREQUENCY          (32000)
#define LEDC_PERIOD             (2048)


void initLedC(void);
void setLedC(int gpio, unsigned channel, float value, bool invert=0);

