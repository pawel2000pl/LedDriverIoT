#include <functional>
#include <cstring>

#include "threads_mgr.h"

#include "knobs.h"
#include "timer_shutdown.h"
#include "animations.h"

#define TASK_NAME_SIZE 16

namespace threads_mgr {
    
    volatile bool settingsInLock = false;

    void setLock(bool lockState) {
        settingsInLock = lockState;
    }

    class TaskThread {

        public:

            TaskThread(std::function<void()> handler, unsigned long int period=16, const char* name="")
             : runner(handler), period_ms(period), taskHandle(NULL) {
                unsigned len = strlen(name);
                if (len > TASK_NAME_SIZE) len = TASK_NAME_SIZE;
                for (unsigned i=0;i<len;i++)
                    task_name[i] = name[i];
                task_name[len] = 0;
                task_name[TASK_NAME_SIZE] = 0;
             }

            void init() {
                if (taskHandle) return;
                xTaskCreatePinnedToCore(
                    TaskThread::loop,     // Task function
                    task_name,            // Task name
                    2048,                 // Stack size
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
            char task_name[TASK_NAME_SIZE+1];

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

    TaskThread knobsThread(knobs::checkTimer, 20, "knobs");
    TaskThread shutdownThread(timer_shutdown::checkTimer, 20, "shutdown");
    TaskThread animationsThread(animations::checkTimer, 20, "animations");


    void attachTimer() {
        knobsThread.init();
        shutdownThread.init();
        animationsThread.init();
    }

}
