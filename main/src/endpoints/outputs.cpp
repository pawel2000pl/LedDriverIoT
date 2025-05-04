#include "outputs.h"

#include "../outputs.h"

namespace endpoints {

    void getTailoredScalling(HTTPRequest* req, HTTPResponse* res) {
        ColorChannels scalling = outputs::getTailoredScalling();
        char buf[64];
        int size = 0;
        buf[size++] = '[';
        size += scalling[0].toCharBuf(buf+size, 10, 9);
        buf[size++] = ',';
        size += scalling[1].toCharBuf(buf+size, 10, 9);
        buf[size++] = ',';
        size += scalling[2].toCharBuf(buf+size, 10, 9);
        buf[size++] = ',';
        size += scalling[3].toCharBuf(buf+size, 10, 9);
        buf[size++] = ']';
        buf[size] = 0;
        char size_str[24];
        res->setHeader("Cache-Control", "no-cache");
        res->setHeader("Content-Type", "application/json");
        res->setHeader("Content-Length", itoa(size, size_str, 10));
        res->write((uint8_t*)buf, size);
    }

}
