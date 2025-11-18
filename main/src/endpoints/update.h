#pragma once

#include "../lib/esp32_https_server/HTTPRequest.hpp"
#include "../lib/esp32_https_server/HTTPResponse.hpp"

namespace endpoints {

    using HTTPResponse = httpsserver::HTTPResponse;
    using HTTPRequest = httpsserver::HTTPRequest;

    void update(HTTPRequest* req, HTTPResponse* res);
    void restart(HTTPRequest* req, HTTPResponse* res);

}
