#pragma once

#define CONNECTION_TIMEOUT 15000

#include <Arduino.h>
#include <vector>
#include "lib/ArduinoJson/ArduinoJson.h"
#include "json_utils.h"

namespace wifi {

    struct WiFiConfigEntry {
        String ssid;
        String password;
        bool hidden;
        bool periodicScan;
    };

    const std::vector<String>& getScannedNetworks();
    IPAddress str2ip(const String& str);
    void activity();
    void updateConfiguration(const JsonVariantConst configuration);
    void scanNetworks();
    void disconnect();
    bool connectToNetwork(const WiFiConfigEntry& entry);
    void fastInit();
    bool autoSta();
    bool connected();
    void openAccessPoint();
    void autoConnectWifi();
    void checkConnection();
    bool isAP();
    String getApAddress();
    String getLocalIp();
    String getHostname();
    unsigned long long int getConnectionTime();
    unsigned getConnectionId();

}
