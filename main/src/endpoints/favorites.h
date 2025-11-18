#pragma once

#include "../lib/esp32_https_server/HTTPRequest.hpp"
#include "../lib/esp32_https_server/HTTPResponse.hpp"

namespace endpoints {

    using HTTPResponse = httpsserver::HTTPResponse;
    using HTTPRequest = httpsserver::HTTPRequest;

    void sendFavorites(HTTPRequest* req, HTTPResponse* res);
    void dumpFavorite(HTTPRequest* req, HTTPResponse* res);
    void saveFavorites(HTTPRequest* req, HTTPResponse* res);
    void applyFavorite(HTTPRequest* req, HTTPResponse* res);
    void renderFavoriteColor(HTTPRequest* req, HTTPResponse* res);

}
