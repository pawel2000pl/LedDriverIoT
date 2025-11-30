#include "initialization.h"

#include <list>
#include <string>
#include <Arduino.h>

#include "../lib/ArduinoJson/ArduinoJson.h"

#include "update.h"
#include "colors.h"
#include "outputs.h"
#include "network.h"
#include "favorites.h"
#include "animations.h"
#include "statistics.h"
#include "temperature.h"
#include "configuration.h"
#include "initialization.h"

#include "../server.h"

namespace endpoints {

    void handleIndex(HTTPRequest* req, HTTPResponse* res) {
        const char* buf = "<meta http-equiv=\"refresh\" content=\"0; url=/index.html\">";
        char size_str[24];
        int size = strlen(buf);
        res->setHeader("Content-Type", "text/html");
        res->setHeader("Content-Length", itoa(size, size_str, 10));
        res->write((uint8_t*)buf, size);
    }


    void sendFavicon(HTTPRequest* req, HTTPResponse* res) {
        server::sendDecompressedData(res, resource_favicon_svg);
    }


    void configureServer() {
        server::configure();	
        server::addCallback("/", "GET", handleIndex);
        server::addCallback("/statistics.txt", "GET", sendStatistics);
        server::addCallback("/config.json", "GET", sendConfiguration);
        server::addCallback("/favicon.ico", "GET", sendFavicon);
        server::addCallback("/config.json", "POST", recvConfiguration);
        server::addCallback("/assert_json", "POST", customValidator);
        server::addCallback("/networks.json", "GET", sendNetworks);
        server::addCallback("/connect_to", "POST", connectToNetworkEndpoint);
        server::addCallback("/refresh_networks", "GET", autoScanWifiEndpoint);
        server::addCallback("/open_access_point", "GET", openAccessPointEndpoint);
        server::addCallback("/reconnect", "GET", reconnect);
        server::addCallback("/color.json", "GET", getColors);
        server::addCallback("/color.json", "POST", setColors);
        server::addCallback("/simple.html", "GET", simpleMode);
        server::addCallback("/simple.html", "POST", simpleMode);
        server::addCallback("/update", "POST", update);
        server::addCallback("/restart", "GET", restart);
        server::addCallback("/version_info.json", "GET", getVersionInfo);
        server::addCallback("/favorite_color.html", "GET", renderFavoriteColor);
        server::addCallback("/get_favorites", "GET", sendFavorites);
        server::addCallback("/new_favorite", "GET", dumpFavorite);
        server::addCallback("/save_favorites", "POST", saveFavorites);
        server::addCallback("/apply_favorite", "GET", applyFavorite);
        server::addCallback("/invalidate_cache", "GET", invalidateCache);
        server::addCallback("/delete_cert", "GET", deleteCert);
        server::addCallback("/get_temp", "GET", getTemperature);
        server::addCallback("/get_tailored_scalling", "GET", getTailoredScalling);
        server::addCallback("/animations.json", "GET", sendAnimations);
        server::addCallback("/animations.json", "POST", saveAnimations);
        server::addCallback("/start_animation", "POST", startAnimation);
        server::start();
    }

}
