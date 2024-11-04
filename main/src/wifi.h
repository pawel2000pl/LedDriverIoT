#pragma once

#define CONNECTION_TIMEOUT 15000

#include <Arduino.h>
#include <ArduinoJson.h>
#include <vector>

namespace wifi {

    const std::vector<String>& getScannedNetworks();
    IPAddress str2ip(const String& str);
    void updateConfiguration(const JsonVariant& configuration);
    void scanNetworks();
    bool connectToNetwork(String ssid, String password);
    void fastInit();
    bool autoSta();
    bool connected();
    void openAccessPoint();
    void autoConnectWifi();
    void checkConnection();

}