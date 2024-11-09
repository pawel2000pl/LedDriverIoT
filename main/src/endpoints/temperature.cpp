#include "temperature.h"

#include <ArduinoJson.h>
#include "../temperature.h"
#include "../server.h"

namespace endpoints {

    void getTemperature(HTTPRequest* req, HTTPResponse* res) {
        temperature::TemperatureResults reads = temperature::readTemperatures();

        DynamicJsonDocument tempJson(1024);
        tempJson["internal"] = reads.internal;
        tempJson["max"] = reads.max();
        tempJson["external"].add(reads.external[0]);
        tempJson["external"].add(reads.external[1]);
        tempJson["external"].add(reads.external[2]);
        tempJson["external"].add(reads.external[3]);
        server::sendJson(res, tempJson);
    }

}
