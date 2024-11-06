#pragma once

#include <HTTPRequest.hpp>
#include <HTTPResponse.hpp>

namespace endpoints {

    using HTTPResponse = httpsserver::HTTPResponse;
    using HTTPRequest = httpsserver::HTTPRequest;

    void connectToNetworkEndpoint(HTTPRequest* req, HTTPResponse* res);
    void autoScanWifiEndpoint(HTTPRequest* req, HTTPResponse* res);
    void openAccessPointEndpoint(HTTPRequest* req, HTTPResponse* res);
    void reconnect(HTTPRequest* req, HTTPResponse* res);

}
