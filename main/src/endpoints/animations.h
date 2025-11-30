#pragma once

#include "../lib/esp32_https_server/HTTPRequest.hpp"
#include "../lib/esp32_https_server/HTTPResponse.hpp"

namespace endpoints {

    using HTTPResponse = httpsserver::HTTPResponse;
    using HTTPRequest = httpsserver::HTTPRequest;

    void sendAnimations(HTTPRequest* req, HTTPResponse* res);
    void saveAnimations(HTTPRequest* req, HTTPResponse* res);
    void startAnimation(HTTPRequest* req, HTTPResponse* res);

}
