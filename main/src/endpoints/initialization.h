#pragma once

#include <HTTPRequest.hpp>
#include <HTTPResponse.hpp>

namespace endpoints {

    using HTTPResponse = httpsserver::HTTPResponse;
    using HTTPRequest = httpsserver::HTTPRequest;

    void handleIndex(HTTPRequest* req, HTTPResponse* res);
    void sendFavicon(HTTPRequest* req, HTTPResponse* res);
    void configureServer();

}
