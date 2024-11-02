#pragma once

#include <Arduino.h>
#include <ArduinoJson.h>
#include <string>
#include <vector>
#include <HTTPSServer.hpp>
#include <HTTPServer.hpp>
#include <SSLCert.hpp>
#include <HTTPRequest.hpp>
#include <HTTPResponse.hpp>
#include <HTTPResponse.hpp>

#include "resources.h"

#define MAX_POST_SIZE (24*1024)

using HTTPResponse = httpsserver::HTTPResponse;
using HTTPRequest = httpsserver::HTTPRequest;
using ResourceNode = httpsserver::ResourceNode;


namespace server {

    using CallbackFunction = httpsserver::HTTPSCallbackFunction;
    
    void addCallback(const char* address, const char* method, const CallbackFunction* callback);
    void configure();
    void start();
    void loop();

    void sendJson(HTTPResponse* res, const JsonVariantConst& data, unsigned bufSize = 1024, int costatusCode = 200);
    void sendError(HTTPResponse* res, String message, int code);
    void sendOk(HTTPResponse* res);
    void sendCacheControlHeader(HTTPResponse* res);    
    int sendDecompressedData(HTTPResponse* res, const Resource& resource, int statusCode=200);
    bool sendDeserializationError(HTTPResponse* res, DeserializationError err);
    void sendResource(HTTPRequest* req, HTTPResponse* res);
    std::vector<char> readBuffer(HTTPRequest* req);
    bool readJson(HTTPRequest* req, HTTPResponse* res, JsonDocument& json, const String& assertEntryName="");

}
