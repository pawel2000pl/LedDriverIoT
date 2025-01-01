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
        JsonDocument testJson;
        if (!server::readJson(req, res, testJson)) return;
        const auto configSchema = configuration::getConfigSchema();
        const String assertMessage = 
            testJson["type"].is<JsonString>() ? 
            validateJson(testJson["data"], configSchema, configSchema[testJson["type"].as<String>()]) : 
            validateJson(testJson["data"], testJson["schema"], testJson["schema"]["main"]);
        if (assertMessage != "") {
            server::sendError(res, assertMessage, 422);
            return;
        }
        server::sendOk(res);
    }


    void recvConfiguration(HTTPRequest* req, HTTPResponse* res) {
        JsonDocument configuration;
        if (!server::readJson(req, res, configuration, "main")) return;
        configuration::setConfiguration(configuration);
        modules::updateModules(configuration);
        server::sendOk(res);
    }


    void invalidateCache(HTTPRequest* req, HTTPResponse* res) {
        res->setHeader("Clear-Site-Data", "\"*\"");
        res->setHeader("Content-Type", "text/html");
        res->setHeader("Cache-Control", "no-cache");
        res->print(
            "<style>\n"
            "html, body {\n"
            "    max-width: 100wv;\n"
            "    font-family: Arial;\n"
            "    text-align: center;\n"
            "    @media (prefers-color-scheme: dark) {\n"
            "        color: white;\n"
            "        background: black;\n"
            "    }\n"
            "}\n"
            "</style>\n"
            "<p>Please wait, upgrading client app...</p>\n"
            "<progress id=\"upgrade_progress\" value=\"0\" max=\"100\"/>\n"
            "<script defer>\n"
            "   async function upgrade() {\n"
        );
        for (unsigned i=0;i<resources_count;i++) {
            res->print("       await (await fetch('");
            res->print(resources[i]->name);
            res->print("', {cache: 'reload'})).text();\n");
            res->print("       document.getElementById('upgrade_progress').value = ");
            res->print((int)((i+1) * 100 / resources_count));
            res->print(";\n");
        }
        res->print(
            "       localStorage.removeItem('version');\n"
            "       history.go(-1);\n"
            "   }\n"
            "   upgrade();"
            "</script>\n"
        );
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
        JsonDocument networkList;
        for (auto s : scannedNetworks) networkList.add(s);
        server::sendJson(res, networkList, bufSize);
    }

}
