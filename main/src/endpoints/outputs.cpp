#include "outputs.h"

#include "../outputs.h"

namespace endpoints {

    void getTailoredScalling(HTTPRequest* req, HTTPResponse* res) {
        ColorChannels scalling = outputs::getTailoredScalling();
        char buf[256];
        int size = sprintf(buf, "[%f, %f, %f, %f]", 
            scalling[0],
            scalling[1],
            scalling[2],
            scalling[3]
        );
        buf[size] = 0;
        char size_str[24];
        res->setHeader("Cache-Control", "no-cache");
        res->setHeader("Content-Type", "application/json");
        res->setHeader("Content-Length", itoa(size, size_str, 10));
        res->write((uint8_t*)buf, size);
    }

}
