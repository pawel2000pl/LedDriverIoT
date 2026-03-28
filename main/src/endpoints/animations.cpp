#include "animations.h"

#include "../server.h"
#include "../animations.h"
#include "../configuration.h"
#include "../timer_shutdown.h"

namespace endpoints {


    void getAnimationLightness(HTTPRequest* req, HTTPResponse* res) {
        const fixed32_c lightness = animations::getGlobalLightness();
        char buf[64];
        int size = lightness.toCharBuf(buf, 10, 12);
        char size_str[24];
        res->setHeader("Cache-Control", "no-cache");
        res->setHeader("Content-Type", "application/json");
        res->setHeader("Content-Length", itoa(size, size_str, 10));
        res->write((uint8_t*)buf, size);
    }


    void setAnimationLightness(HTTPRequest* req, HTTPResponse* res) {
        JsonDocument data;
        if (!server::readJson(req, res, data)) return;
        animations::setGlobalLightness(data.as<fixed32_c>());
        timer_shutdown::resetTimer();
        server::sendOk(res);   
    }


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
        timer_shutdown::resetTimer();
        if (!server::readJson(req, res, data, "animation-start")) return;
        server::sendOk(res);
        animations::startAnimation(data["id"].as<unsigned>());
    }


    void testAnimation(HTTPRequest* req, HTTPResponse* res) {
        JsonDocument data;
        timer_shutdown::resetTimer();
        if (!server::readJson(req, res, data, "animation-sequence")) return;
        server::sendOk(res);
        animations::startAnimationFromJson(data);
    }


}
