
#include "favorites.h"

#include <string>
#include <Arduino.h>
#include <ArduinoJson.h>

#include "../knobs.h"
#include "../server.h"
#include "../inputs.h"
#include "../outputs.h"
#include "../modules.h"
#include "../conversions.h"
#include "../configuration.h"

namespace endpoints {

    void sendFavorites(HTTPRequest* req, HTTPResponse* res) {
        DynamicJsonDocument favorites = configuration::getFavorites();
        String colorspace = modules::webColorSpace;
        DynamicJsonDocument response(JSON_FAVORITES_BUF_SIZE);
        JsonArray reponseArray = response.to<JsonArray>();
        unsigned size = favorites.size();
        for (unsigned i=0;i<size;i++) {
            JsonObject item = reponseArray.createNestedObject();
            String code = favorites[i].as<String>();
            item["code"] = code;
            ColorChannels channels = inputs::favoriteColorPreview(colorspace, code);
            JsonArray colorJson = item.createNestedArray("color");
            colorJson.add(channels[0]);
            colorJson.add(channels[1]);
            colorJson.add(channels[2]);
            colorJson.add(channels[3]);
        }
        server::sendJson(res, response, JSON_FAVORITES_BUF_SIZE);
    }


    void dumpFavorite(HTTPRequest* req, HTTPResponse* res) {
        std::string white = "0";
        req->getParams()->getQueryParameter("white", white);
        bool useWhite = white != "0";
        String dumped = inputs::dumpFavoriteColor(useWhite);
        ColorChannels channels = inputs::getAuto(modules::webColorSpace);
        char buf[256];
        int size = sprintf(buf, "{\"code\": \"%s\", \"color\": [%f, %f, %f, %f]}", 
            dumped.c_str(),
            channels[0],
            channels[1],
            channels[2],
            channels[3]
        );
        buf[size] = 0;
        char size_str[24];
        res->setHeader("Cache-Control", "no-cache");
        res->setHeader("Content-Type", "application/json");
        res->setHeader("Content-Length", itoa(size, size_str, 10));
        res->write((uint8_t*)buf, size);
    }


    void saveFavorites(HTTPRequest* req, HTTPResponse* res) {
        DynamicJsonDocument data(JSON_FAVORITES_BUF_SIZE);
        if (!server::readJson(req, res, data, "favorites-list")) return;
        configuration::setFavorites(data);
        server::sendOk(res);
    }


    void applyFavorite(HTTPRequest* req, HTTPResponse* res) {
        std::string code = "000000000";
        req->getParams()->getQueryParameter("code", code);
        knobs::turnOff();
        inputs::applyFavoriteColor(String(code.c_str()));
        outputs::writeOutput();
        server::sendOk(res);
    }


    void renderFavoriteColor(HTTPRequest* req, HTTPResponse* res) {
        String templateStr = configuration::getResourceStr(resource_favorite_color_template_html);
        std::string code = "000000000";
        req->getParams()->getQueryParameter("code", code);
        inputs::applyFavoriteColor(String(code.c_str()));
        outputs::writeOutput();
        knobs::turnOff();

        const auto& channelsMode = modules::webColorSpace;
        ColorChannels filteredChannels = inputs::getAuto(channelsMode);

        float r, g, b;
        if (channelsMode == "rgb") rgbToRgb(filteredChannels[0], filteredChannels[1], filteredChannels[2], r, g, b);
        if (channelsMode == "hsl") hslToRgb(filteredChannels[0], filteredChannels[1], filteredChannels[2], r, g, b);
        if (channelsMode == "hsv") hsvToRgb(filteredChannels[0], filteredChannels[1], filteredChannels[2], r, g, b);
        char* render_buffer = new char[favorite_color_template_html_decompressed_size+256];
        int size = sprintf(
            render_buffer, templateStr.c_str(),
            (int)floor(255*r), (int)floor(255*g), (int)floor(255*b)
        );
        render_buffer[size] = 0;
        char size_str[24];
        res->setHeader("Cache-Control", "no-cache");
        res->setHeader("Content-Type", "text/html");
        res->setHeader("Content-Length", itoa(size, size_str, 10));
        res->write((uint8_t*)render_buffer, size);
        delete [] render_buffer;
    }

}
