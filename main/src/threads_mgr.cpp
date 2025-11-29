#include <functional>

#include "threads_mgr.h"

#include "knobs.h"
#include "timer_shutdown.h"
#include "animations.h"

namespace threads_mgr {
    
    volatile bool settingsInLock = false;

    void setLock(bool lockState) {
        settingsInLock = lockState;
    }

    class TaskThread {

        public:

            TaskThread(std::function<void()> handler, unsigned long int period=16)
             : runner(handler), period_ms(period), taskHandle(NULL) {}

            void init() {
                if (taskHandle) return;
                xTaskCreatePinnedToCore(
                    TaskThread::loop,     // Task function
                    "unnamed",            // Task name
                    10000,                // Stack size (bytes)
                    (void*)this,          // Parameters
                    1,                    // Priority
                    &taskHandle,          // Task handle
                    0                     // Core 0
                );
            }

        private:

            std::function<void()> runner;
            unsigned long int period_ms;
            TaskHandle_t taskHandle;

            void execute() {
                if (!settingsInLock)
                    runner();                
                vTaskDelay(std::max<long int>(period_ms - (millis() % period_ms), (period_ms >> 1) + 1) / portTICK_PERIOD_MS);
            }

            static void loop(void* instance) {
                while (1) {
                    ((TaskThread*)instance)->execute();
                }
            }

    };

    TaskThread knobsThread(knobs::checkTimer, 20);
    TaskThread shutdownThread(timer_shutdown::checkTimer, 20);
    TaskThread animationsThread(animations::checkTimer, 20);


    void attachTimer() {
        knobsThread.init();
        shutdownThread.init();
        animationsThread.init();
    }

}
