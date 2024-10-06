#include <hal/ledc_types.h>

#define LEDC_TIMER              LEDC_TIMER_0
#define LEDC_MODE               LEDC_LOW_SPEED_MODE
#define LEDC_DUTY_RES           LEDC_TIMER_11_BIT
#define LEDC_PERIOD             (2048)

void initLedC(void);
void checkNewFrequency(unsigned freq);
float addGateLoadingTime(float value, float loadingTime);
void setLedC(int gpio, unsigned channel, float value, bool invert=0);

