#include "configuration.h"

#include <Arduino.h>
#include <ArduinoJson.h>

#include "../wifi.h"
#include "../server.h"
#include "../modules.h"
#include "../configuration.h"
#include "../validate_json.h"


namespace endpoints {

    void sendConfiguration(HTTPRequest* req, HTTPResponse* res) {
        String buf = configuration::getConfigurationStr();
        char size_str[24];
        int size = buf.length();
        res->setHeader("Cache-Control", "no-cache");
        res->setHeader("Content-Type", "application/json");
        res->setHeader("Content-Length", itoa(size, size_str, 10));
        res->setStatusCode(200);
        res->write((uint8_t*)buf.c_str(), size);
    }


    void customValidator(HTTPRequest* req, HTTPResponse* res) {
        DynamicJsonDocument testJson(JSON_CONFIG_BUF_SIZE);
        if (!server::readJson(req, res, testJson)) return;
        const auto configSchema = configuration::getConfigSchema();
        const String assertMessage = 
            testJson.containsKey("type") ? 
            validateJson(testJson["data"], configSchema, configSchema[testJson["type"].as<String>()]) : 
            validateJson(testJson["data"], testJson["schema"], testJson["schema"]["main"]);
        if (assertMessage != "") {
            server::sendError(res, assertMessage, 422);
            return;
        }
        server::sendOk(res);
    }


    void recvConfiguration(HTTPRequest* req, HTTPResponse* res) {
        DynamicJsonDocument configuration(JSON_CONFIG_BUF_SIZE);
        if (!server::readJson(req, res, configuration, "main")) return;
        configuration::setConfiguration(configuration);
        modules::updateModules(configuration);
        server::sendOk(res);
    }


    void invalidateCache(HTTPRequest* req, HTTPResponse* res) {
        const char* buf = "<p>Please wait, upgrading client app...</p><script defer>localStorage.removeItem('version'); history.go(-1);</script>";
        char size_str[24];
        int size = strlen(buf);
        res->setHeader("Clear-Site-Data", "\"*\"");
        res->setHeader("Content-Type", "text/html");
        res->setHeader("Content-Length", itoa(size, size_str, 10));
        res->setHeader("Connection", "close");
        res->write((uint8_t*)buf, size);
    }


    void getVersionInfo(HTTPRequest* req, HTTPResponse* res) {
        server::sendJson(res, configuration::getVersionInfo());
    }


    void deleteCert(HTTPRequest* req, HTTPResponse* res) {
        server::sendOk(res); 
        configuration::deleteCert();
    }


    void sendNetworks(HTTPRequest* req, HTTPResponse* res) {
        unsigned lengthSum = 0;
        const auto& scannedNetworks = wifi::getScannedNetworks();
        for (auto s : scannedNetworks) lengthSum += s.length();
        unsigned bufSize = lengthSum + 64 * scannedNetworks.size() + 32;
        DynamicJsonDocument networkList(bufSize);
        for (auto s : scannedNetworks) networkList.add(s);
        server::sendJson(res, networkList, bufSize);
    }

}
