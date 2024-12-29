#include "modules.h"

#include "wifi.h"
#include "knobs.h"
#include "server.h"
#include "inputs.h"
#include "outputs.h"
#include "configuration.h"

namespace modules {

    std::list<std::function<void()>> taskQueue;
    String webColorSpace;
    bool whiteKnobEnabled;


    void updateModules(JsonVariant configuration) {
        knobs::setLock(true);
        knobs::updateConfiguration(configuration);
        inputs::updateConfiguration(configuration);
        outputs::updateConfiguration(configuration);
        wifi::updateConfiguration(configuration);
        server::updateConfiguration(configuration);
        webColorSpace = configuration["channels"]["webMode"].as<String>();
        whiteKnobEnabled = configuration["hardware"]["enbleWhiteKnob"].as<bool>();
        knobs::setLock(false);
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
