#include "modules.h"

#include "wifi.h"
#include "knobs.h"
#include "server.h"
#include "inputs.h"
#include "outputs.h"
#include "threads_mgr.h"
#include "configuration.h"
#include "timer_shutdown.h"

namespace modules {

    std::list<std::function<void()>> taskQueue;
    String webColorSpace;
    bool colorKnobEnabled;
    bool whiteKnobEnabled;


    void updateModules(JsonVariant configuration) {
        threads_mgr::setLock(true);
        knobs::updateConfiguration(configuration);
        inputs::updateConfiguration(configuration);
        outputs::updateConfiguration(configuration);
        wifi::updateConfiguration(configuration);
        server::updateConfiguration(configuration);
        timer_shutdown::updateConfiguration(configuration);
        webColorSpace = configuration["channels"]["webMode"].as<String>();
        colorKnobEnabled = configuration["hardware"]["enableColorKnob"].as<bool>();
        whiteKnobEnabled = configuration["hardware"]["enableWhiteKnob"].as<bool>();
        threads_mgr::setLock(false);
    }


    void updateModules() {
        auto configuration = configuration::getConfiguration();
        updateModules(configuration);
    }


    void execTaskQueue() {
        for (auto fun: taskQueue) 
            fun();
        taskQueue.clear();
    }

}
