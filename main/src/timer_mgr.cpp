#include "timer_mgr.h"

#include "knobs.h"
#include "timer_shutdown.h"

namespace timer_mgr {
    
    hw_timer_t * timer = NULL;
    bool settingsInLock = false;
    bool timerExecution = false;

    void setLock(bool lockState) {
        settingsInLock = lockState;
    }


    void ARDUINO_ISR_ATTR timerCheck60() {
        if (settingsInLock || timerExecution) return;
        timerExecution = true;
        try {
            knobs::checkTimer();
            timer_shutdown::checkTimer();
            timerExecution = false;
        } catch (...) {
            timerExecution = false;
        }
    }


    void attachTimer() {
        if (timer) return;
        timer = timerBegin(1000000);
        timerAttachInterrupt(timer, &timerCheck60);
        timerAlarm(timer, 20000, true, 0);
    }

}
