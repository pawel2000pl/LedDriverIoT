#pragma once

#include "../lib/esp32_https_server/HTTPRequest.hpp"
#include "../lib/esp32_https_server/HTTPResponse.hpp"

namespace endpoints {

    using HTTPResponse = httpsserver::HTTPResponse;
    using HTTPRequest = httpsserver::HTTPRequest;

    void connectToNetworkEndpoint(HTTPRequest* req, HTTPResponse* res);
    void autoScanWifiEndpoint(HTTPRequest* req, HTTPResponse* res);
    void openAccessPointEndpoint(HTTPRequest* req, HTTPResponse* res);
    void reconnect(HTTPRequest* req, HTTPResponse* res);

}
