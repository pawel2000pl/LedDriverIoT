#pragma once

#include <HTTPRequest.hpp>
#include <HTTPResponse.hpp>

namespace endpoints {

    using HTTPResponse = httpsserver::HTTPResponse;
    using HTTPRequest = httpsserver::HTTPRequest;

    void getColors(HTTPRequest* req, HTTPResponse* res);
    void setColors(HTTPRequest* req, HTTPResponse* res);
    void simpleMode(HTTPRequest* req, HTTPResponse* res);

}
