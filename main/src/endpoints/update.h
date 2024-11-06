#pragma once

#include <HTTPRequest.hpp>
#include <HTTPResponse.hpp>

namespace endpoints {

    using HTTPResponse = httpsserver::HTTPResponse;
    using HTTPRequest = httpsserver::HTTPRequest;

    void update(HTTPRequest* req, HTTPResponse* res);
    void restart(HTTPRequest* req, HTTPResponse* res);

}
