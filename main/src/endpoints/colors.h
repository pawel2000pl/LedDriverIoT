#pragma once

#include "../lib/esp32_https_server/HTTPRequest.hpp"
#include "../lib/esp32_https_server/HTTPResponse.hpp"

namespace endpoints {

    using HTTPResponse = httpsserver::HTTPResponse;
    using HTTPRequest = httpsserver::HTTPRequest;

    void getColors(HTTPRequest* req, HTTPResponse* res);
    void setColors(HTTPRequest* req, HTTPResponse* res);
    void simpleMode(HTTPRequest* req, HTTPResponse* res);

}
