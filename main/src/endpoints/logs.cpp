#include "logs.h"
#include "../logs.h"

#include <algorithm>

namespace endpoints {

    using HTTPResponse = httpsserver::HTTPResponse;
    using HTTPRequest = httpsserver::HTTPRequest;


    void sendLogs(HTTPRequest* req, HTTPResponse* res) {
        char size_str[24];
        res->setHeader("Content-Length", itoa(logs::logger.buf_size, size_str, 10));
        res->setHeader("Content-Type", "text/plain");
        res->setHeader("Content-Disposition", "inline");
        unsigned char buf[256];
        unsigned rest = logs::logger.buf_size;
        while (rest) {
            unsigned readed = logs::logger.get_buf(buf, logs::logger.buf_size - rest, std::min<unsigned>(256, rest));
            res->write(buf, readed);
            rest -= readed;
        }
    }

}
