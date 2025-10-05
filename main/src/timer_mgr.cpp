#include "timer_mgr.h"

#include "knobs.h"
#include "timer_shutdown.h"

namespace timer_mgr {
    
    hw_timer_t * timer = NULL;
    bool settingsInLock = false;


    void setLock(bool lockState) {
        settingsInLock = lockState;
    }


    void ARDUINO_ISR_ATTR timerCheck60() {
        if (settingsInLock) return;
        knobs::checkTimer();
        timer_shutdown::checkTimer();
    }


    void attachTimer() {
        if (timer) return;
        timer = timerBegin(1000000);
        timerAttachInterrupt(timer, &timerCheck60);
        timerAlarm(timer, 16667, true, 0);
    }

}
