#include "timer_mgr.h"

#include "knobs.h"
#include "timer_shutdown.h"

namespace timer_mgr {
    
    hw_timer_t * timer = NULL;
    volatile bool settingsInLock = false;
    volatile bool timerExecution = false;
    TaskHandle_t taskHandle = NULL;


    void setLock(bool lockState) {
        settingsInLock = lockState;
    }


    void timerCheck60(void*) {
        while (1) {
            if (!(settingsInLock || timerExecution)) {
                timerExecution = true;
                knobs::checkTimer();
                timer_shutdown::checkTimer();
                timerExecution = false;
            }
            vTaskDelay(20 / portTICK_PERIOD_MS);
        }
    }


    void attachTimer() {
        if (taskHandle) return;
        xTaskCreatePinnedToCore(
            timerCheck60,         // Task function
            "timerCheck60",       // Task name
            10000,                // Stack size (bytes)
            NULL,                 // Parameters
            1,                    // Priority
            &taskHandle,          // Task handle
            0                     // Core 0
        );
    }

}
