#pragma once

#include <HTTPRequest.hpp>
#include <HTTPResponse.hpp>

namespace endpoints {

    using HTTPResponse = httpsserver::HTTPResponse;
    using HTTPRequest = httpsserver::HTTPRequest;

    void sendFavorites(HTTPRequest* req, HTTPResponse* res);
    void dumpFavorite(HTTPRequest* req, HTTPResponse* res);
    void saveFavorites(HTTPRequest* req, HTTPResponse* res);
    void applyFavorite(HTTPRequest* req, HTTPResponse* res);
    void renderFavoriteColor(HTTPRequest* req, HTTPResponse* res);

}
