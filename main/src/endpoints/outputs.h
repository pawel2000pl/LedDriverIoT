#pragma once

#include <HTTPRequest.hpp>
#include <HTTPResponse.hpp>

namespace endpoints {

    using HTTPResponse = httpsserver::HTTPResponse;
    using HTTPRequest = httpsserver::HTTPRequest;

    void getTailoredScalling(HTTPRequest* req, HTTPResponse* res);

}
