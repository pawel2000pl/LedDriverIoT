#pragma once

#include <Arduino.h>
#include <string>
#include <vector>
#include "lib/esp32_https_server/HTTPSServer.hpp"
#include "lib/esp32_https_server/HTTPServer.hpp"
#include "lib/esp32_https_server/SSLCert.hpp"
#include "lib/esp32_https_server/HTTPRequest.hpp"
#include "lib/esp32_https_server/HTTPResponse.hpp"

#include "lib/ArduinoJson/ArduinoJson.h"
#include "resources.h"
#include "json_utils.h"

#define MAX_POST_SIZE (16*1024-1)

using HTTPResponse = httpsserver::HTTPResponse;
using HTTPRequest = httpsserver::HTTPRequest;
using ResourceNode = httpsserver::ResourceNode;

namespace server {

    using CallbackFunction = httpsserver::HTTPSCallbackFunction;
    
    void updateConfiguration(const JsonVariantConst& configuration);
    
    void addCallback(const char* address, const char* method, const CallbackFunction* callback);
    void configure();
    void start();
    void loop();
    void stop();
    void restart();

    void sendJson(HTTPResponse* res, const JsonVariantConst& data, unsigned bufSize = 4096, int statusCode = 200);
    void sendError(HTTPResponse* res, String message, int code);
    void sendOk(HTTPResponse* res);
    void sendCacheControlHeader(HTTPResponse* res);    
    int sendDecompressedData(HTTPResponse* res, const Resource& resource, int statusCode=200);
    bool sendDeserializationError(HTTPResponse* res, DeserializationError err);
    void sendResource(HTTPRequest* req, HTTPResponse* res);
    std::vector<char>* readBuffer(HTTPRequest* req, bool addZero=true);
    bool readJson(HTTPRequest* req, HTTPResponse* res, JsonDocument& json, const String& assertEntryName="");

}
