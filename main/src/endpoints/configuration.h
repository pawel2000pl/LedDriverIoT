#pragma once

#include <HTTPRequest.hpp>
#include <HTTPResponse.hpp>

namespace endpoints {

    using HTTPResponse = httpsserver::HTTPResponse;
    using HTTPRequest = httpsserver::HTTPRequest;

    void sendConfiguration(HTTPRequest* req, HTTPResponse* res);
    void customValidator(HTTPRequest* req, HTTPResponse* res);
    void recvConfiguration(HTTPRequest* req, HTTPResponse* res);
    void invalidateCache(HTTPRequest* req, HTTPResponse* res);
    void getVersionInfo(HTTPRequest* req, HTTPResponse* res);
    void sendNetworks(HTTPRequest* req, HTTPResponse* res);
    void deleteCert(HTTPRequest* req, HTTPResponse* res);

}
