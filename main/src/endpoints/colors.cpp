#include "colors.h"

#include <list>
#include <string>
#include <Arduino.h>

#include "../lib/ArduinoJson/ArduinoJson.h"

#include "../knobs.h"
#include "../server.h"
#include "../inputs.h"
#include "../outputs.h"
#include "../modules.h"
#include "../constrain.h"
#include "../common_types.h"
#include "../configuration.h"
#include "../timer_shutdown.h"
#include "../lib/esp32_https_server/HTTPURLEncodedBodyParser.hpp"

namespace endpoints {

    int fixedpointFilter15(fixed32_c x) { 
        return (int)std::round(x * 15); 
    };

    void getColors(HTTPRequest* req, HTTPResponse* res) {
        std::string colorspace = "";
        ColorChannels channels = inputs::getAuto(req->getParams()->getQueryParameter("colorspace", colorspace) ? String(colorspace.c_str()) : modules::webColorSpace);
        char buf[64];
        int size = 0;
        buf[size++] = '[';
        size += channels[0].toCharBuf(buf+size, 10, 12);
        buf[size++] = ',';
        size += channels[1].toCharBuf(buf+size, 10, 12);
        buf[size++] = ',';
        size += channels[2].toCharBuf(buf+size, 10, 12);
        buf[size++] = ',';
        size += channels[3].toCharBuf(buf+size, 10, 12);
        buf[size++] = ']';
        buf[size] = 0;
        char size_str[24];
        res->setHeader("Cache-Control", "no-cache");
        res->setHeader("Content-Type", "application/json");
        res->setHeader("Content-Length", itoa(size, size_str, 10));
        res->write((uint8_t*)buf, size);
    }


    void setColors(HTTPRequest* req, HTTPResponse* res) {
        JsonDocument data;
        if (!server::readJson(req, res, data)) return;
        std::string colorspace = "";
        inputs::setAuto(req->getParams()->getQueryParameter("colorspace", colorspace) ? String(colorspace.c_str()) : modules::webColorSpace, {data[0].as<fixed32_c>(), data[1].as<fixed32_c>(), data[2].as<fixed32_c>(), data[3].as<fixed32_c>()});
        knobs::turnOff();
        timer_shutdown::resetTimer();
        outputs::writeOutput();
        server::sendOk(res);   
    }


    void simpleMode(HTTPRequest* req, HTTPResponse* res) {
        if (req->getMethod() == "POST") {            
            ColorChannels channels = {0, 0, 0, 0};
            httpsserver::HTTPURLEncodedBodyParser parser(req);
            while(parser.nextField()) {
                std::string name = parser.getFieldName();
                char buffer[16];
                unsigned size = parser.read((unsigned char*)buffer, 16);
                if (!size) continue;
                buffer[size] = 0;
                fixed32_c value = constrain<fixed64>(fixed64::fromCharBuf(buffer)/15, 0, 1);
                if (name == "ch0") channels[0] = value;
                if (name == "ch1") channels[1] = value;
                if (name == "ch2") channels[2] = value;
                if (name == "ch3") channels[3] = value;
            }
            knobs::turnOff();
            timer_shutdown::resetTimer();
            inputs::setAuto(modules::webColorSpace, channels);
            outputs::writeOutput();
        }
        String templateStr = configuration::getResourceStr(resource_simple_template_html);
        const char* colorspaces[] = {"hsv", "hsl", "rgb"};
        const char* channels[][4] = {{"hue", "saturation", "value", "white"}, {"hue", "saturation", "lightness", "white"}, {"red", "green", "blue", "white"}};
        const char** channelsInCurrentColorspace = channels[0];
        String destColorspace = modules::webColorSpace;
        for (int i=0;i<3;i++)
            if (destColorspace == colorspaces[i])
                channelsInCurrentColorspace = channels[i];
        ColorChannels filteredChannels = inputs::getAuto(modules::webColorSpace);
        char* render_buffer = new char[simple_template_html_decompressed_size+256];
        int size = sprintf(
            render_buffer, templateStr.c_str(), 
            modules::colorKnobEnabled ? "" : "display: none;",
            channelsInCurrentColorspace[0],
            fixedpointFilter15(filteredChannels[0]),
            channelsInCurrentColorspace[1],
            fixedpointFilter15(filteredChannels[1]),
            channelsInCurrentColorspace[2],
            fixedpointFilter15(filteredChannels[2]),
            modules::whiteKnobEnabled ? "" : "display: none;",
            channelsInCurrentColorspace[3],
            fixedpointFilter15(filteredChannels[3])
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
