#include "colors.h"

#include <list>
#include <string>
#include <Arduino.h>
#include <ArduinoJson.h>

#include "../knobs.h"
#include "../server.h"
#include "../inputs.h"
#include "../outputs.h"
#include "../modules.h"
#include "../constrain.h"
#include "../configuration.h"

namespace endpoints {

    int floatFilter15(float x) { 
        return (int)round(x * 15.f); 
    };


    void sendColors(HTTPRequest* req, HTTPResponse* res) {
        ColorChannels channels = inputs::getAuto(modules::webColorSpace);
        char buf[256];
        int size = sprintf(buf, "[%f, %f, %f, %f]", 
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


    void setColors(HTTPRequest* req, HTTPResponse* res) {
        JsonDocument data;
        if (!server::readJson(req, res, data)) return;
        inputs::setAuto(modules::webColorSpace, {data[0].as<float>(), data[1].as<float>(), data[2].as<float>(), data[3].as<float>()});
        outputs::writeOutput();
        knobs::turnOff();
        server::sendOk(res);   
    }


    void simpleMode(HTTPRequest* req, HTTPResponse* res) {
        if (req->getMethod() == "POST") {
            httpsserver::ResourceParameters* params = req->getParams();
            auto fun = [=](std::string name) {
                std::string param = "0000";
                params->getQueryParameter(name, param);
                return constrain<float>((float)atoi(param.c_str())/15.f, 0, 1);
            };    
            ColorChannels channels = {fun("ch0"), fun("ch1"), fun("ch2"), fun("ch3")};
            knobs::turnOff();
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
            channelsInCurrentColorspace[0],
            floatFilter15(filteredChannels[0]),
            channelsInCurrentColorspace[1],
            floatFilter15(filteredChannels[1]),
            channelsInCurrentColorspace[2],
            floatFilter15(filteredChannels[2]),
            modules::whiteKnobEnabled ? "" : "style=\"display: none;\"",
            channelsInCurrentColorspace[3],
            floatFilter15(filteredChannels[3])
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
