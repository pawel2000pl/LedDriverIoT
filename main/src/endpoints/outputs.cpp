#include "outputs.h"

#include "../outputs.h"

namespace endpoints {

    void getTailoredScalling(HTTPRequest* req, HTTPResponse* res) {
        ColorChannels scalling = outputs::getTailoredScalling();
        char buf[64];
        int size = sprintf(buf, "[%f, %f, %f, %f]", 
            (float)scalling[0],
            (float)scalling[1],
            (float)scalling[2],
            (float)scalling[3]
        );
        buf[size] = 0;
        char size_str[24];
        res->setHeader("Cache-Control", "no-cache");
        res->setHeader("Content-Type", "application/json");
        res->setHeader("Content-Length", itoa(size, size_str, 10));
        res->write((uint8_t*)buf, size);
    }

}
