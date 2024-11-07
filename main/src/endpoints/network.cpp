#include "network.h"

#include <Arduino.h>
#include <ArduinoJson.h>

#include "../wifi.h"
#include "../server.h"
#include "../modules.h"
#include "../server.h"

namespace endpoints {

    void connectToNetworkEndpoint(HTTPRequest* req, HTTPResponse* res) {
        StaticJsonDocument<768> data;
        if (!server::readJson(req, res, data, "wifi-entry")) return;
        server::sendOk(res);
        const String ssid = data["ssid"].as<String>();
        const String password = data["password"].as<String>();
        modules::taskQueue.push_back([=](){wifi::connectToNetwork(ssid, password);});
    }


    void autoScanWifiEndpoint(HTTPRequest* req, HTTPResponse* res) {
        server::sendOk(res); 
        modules::taskQueue.push_back(wifi::scanNetworks);
    }


    void openAccessPointEndpoint(HTTPRequest* req, HTTPResponse* res) {
        server::sendOk(res); 
        modules::taskQueue.push_back(wifi::openAccessPoint);
    }


    void reconnect(HTTPRequest* req, HTTPResponse* res) {
        server::sendOk(res); 
        modules::taskQueue.push_back([](){
            wifi::disconnect();
            wifi::autoConnectWifi();
        });
    }


}
