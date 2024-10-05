#include "wifi.h"

#include <WiFi.h>
#include <WiFiAP.h>

#define MAX_WIFI_CHANNEL 13
#define RSSI_AMP 0.2f
#define RSSI_TRESHOLD (-80)
std::vector<unsigned> AP_CHANNELS = {1, 6, 11};

namespace wifi {

    struct WiFiConfigEntry {
        String ssid;
        String password;
        bool hidden;
    };

    std::vector<String> scannedNetworks;
    std::vector<WiFiConfigEntry> wifiConfiguration;
    std::vector<WiFiConfigEntry> staPriority;
    WiFiConfigEntry apConfig;
    IPAddress apAddress, apGateway, apSubnet;
    bool apEnabled = true;
    bool apHidden = false;
    String hostname;

    float occupedChannels[MAX_WIFI_CHANNEL+1] = {0};

    void updateConfiguration(const JsonVariant& configuration) {
        hostname = configuration["wifi"]["hostname"].as<String>();
        const auto& apConfigJson = configuration["wifi"]["access_point"];
        apAddress = str2ip(apConfigJson["address"].as<String>());
        apGateway = str2ip(apConfigJson["gateway"].as<String>());
        apSubnet = str2ip(apConfigJson["subnet"].as<String>());
        apConfig.ssid = apConfigJson["ssid"].as<String>();
        apConfig.password = apConfigJson["password"].as<String>();
        apConfig.hidden = apConfigJson["hidden"].as<String>();
        apEnabled = apConfigJson["enabled"].as<bool>();

        const auto& staPriorityJson = configuration["wifi"]["sta_priority"];
        unsigned size = staPriorityJson.size();
        staPriority.clear();
        staPriority.shrink_to_fit();
        staPriority.reserve(size);
        for (unsigned i=0;i<size;i++) {
            const auto& entryJson = staPriorityJson[i];
            WiFiConfigEntry entry;
            entry.ssid = entryJson["ssid"].as<String>();
            entry.password = entryJson["password"].as<String>();
            entry.hidden = entryJson["hidden"].as<String>();
            staPriority.push_back(entry);
        }

    }


    const std::vector<String>& getScannedNetworks() {
        return scannedNetworks;
    }


    IPAddress str2ip(const String& str) {
        uint8_t numbers[4] = {0, 0, 0, 0};
        int j = 0;
        for (int i=0;i<4;i++) {
            while (1) {
                char currChar = str[j++];
                if (currChar < '0' || currChar > '9') break;
                numbers[i] = numbers[i] * 10 + (currChar - '0');
            }
        }
        return IPAddress(numbers[0], numbers[1], numbers[2], numbers[3]);
    }


    void scanNetworks() {
        for (int i=0;i<=MAX_WIFI_CHANNEL;i++)
            occupedChannels[i] = 0;
        Serial.println("Scanning networks");
        int n = WiFi.scanNetworks();
        scannedNetworks.clear();
        scannedNetworks.shrink_to_fit();
        scannedNetworks.reserve(n);
        for (int i=0;i<n;i++) {
            scannedNetworks.push_back(WiFi.SSID(i));
            unsigned  channel = WiFi.channel(i);
            if (channel <= MAX_WIFI_CHANNEL) 
                occupedChannels[channel] += 1.f / (1.f + exp(RSSI_AMP * (WiFi.RSSI(i) - RSSI_TRESHOLD)));
        }
        WiFi.scanDelete();
    }


    unsigned bestApChannel() {
        float minValue = 0;
        unsigned minChannel = 0;
        for (auto channel : AP_CHANNELS)
            if (occupedChannels[channel] < minValue) {
                minValue = occupedChannels[channel];
                minChannel = channel;
            }
        return minChannel;
    }


    bool connectToNetwork(String ssid, String password) {
        WiFi.mode(WIFI_STA);
        if (WiFi.status() == WL_CONNECTED)
            WiFi.disconnect();
        else if (WiFi.getMode() == WIFI_AP)
            WiFi.softAPdisconnect();
        delay(100);
        Serial.print("Connecting to ");
        Serial.println(ssid);
        unsigned long long int timeout_time = millis() + CONNECTION_TIMEOUT;
        WiFi.setHostname(hostname.c_str());
        WiFi.begin(ssid, password);
        while (WiFi.status() != WL_CONNECTED && WiFi.status() != WL_CONNECT_FAILED && millis() <= timeout_time)
                delay(10);
        return WiFi.status() == WL_CONNECTED;
    }


    void openAccessPoint() {
        Serial.println("Switching to AP-mode");
        if (WiFi.status() == WL_CONNECTED)
            WiFi.disconnect();
        WiFi.mode(WIFI_AP);
        WiFi.softAPsetHostname(hostname.c_str());
        WiFi.softAPConfig(apAddress, apGateway, apSubnet);
        WiFi.softAP(apConfig.ssid, apConfig.password, bestApChannel(), apConfig.hidden, 16);
    }


    bool networkIsInScanned(String name) {
        int size = scannedNetworks.size();
        for (int i=0;i<size;i++)
            if (scannedNetworks[i] == name)
                return true;
        return false;
    }


    void fastInit(bool scan) {
        if (WiFi.status() == WL_CONNECTED) {
            WiFi.mode(WIFI_STA);
            WiFi.disconnect();
        }

        if (scan) scanNetworks();

        WiFi.config(INADDR_NONE, INADDR_NONE, INADDR_NONE, INADDR_NONE);
        WiFi.setHostname(hostname.c_str());
    }
    

    bool autoSta() {
        for (int i=0;i<staPriority.size();i++) {
            WiFi.mode(WIFI_STA);
            const auto& entry = staPriority[i];
            if (entry.hidden || networkIsInScanned(entry.ssid)) {
                if (connectToNetwork(entry.ssid, entry.password))
                    return true;
            }
        }
        return false;
    }


    void autoConnectWifi() {
        fastInit(true);
        if (autoSta()) return;
        if (apEnabled) openAccessPoint();
    }


    void checkConnection() {
		if (WiFi.status() != WL_CONNECTED && WiFi.getMode() != WIFI_AP)
			autoConnectWifi(); 
    }

}
