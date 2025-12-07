#include "animations.h"

#include "../server.h"
#include "../animations.h"
#include "../configuration.h"

namespace endpoints {

    void sendAnimations(HTTPRequest* req, HTTPResponse* res) {
        String buf = configuration::getAnimationsStr();
        char size_str[24];
        int size = buf.length();
        res->setHeader("Cache-Control", "no-cache");
        res->setHeader("Content-Type", "application/json");
        res->setHeader("Content-Length", itoa(size, size_str, 10));
        res->setStatusCode(200);
        res->write((uint8_t*)buf.c_str(), size);
    }


    void saveAnimations(HTTPRequest* req, HTTPResponse* res) {
        JsonDocument data;
        if (!server::readJson(req, res, data, "animations-list")) return;
        configuration::setAnimations(data);
        server::sendOk(res);
    }


    void startAnimation(HTTPRequest* req, HTTPResponse* res) {
        JsonDocument data;
        if (!server::readJson(req, res, data, "animation-start")) return;
        server::sendOk(res);
        animations::startAnimation(data["id"].as<unsigned>());
    }


    void testAnimation(HTTPRequest* req, HTTPResponse* res) {
        JsonDocument data;
        if (!server::readJson(req, res, data, "animation-sequence")) return;
        server::sendOk(res);
        animations::startAnimationFromJson(data);
    }


}
