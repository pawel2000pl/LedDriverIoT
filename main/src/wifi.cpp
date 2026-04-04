#include <WiFi.h>
#include <WiFiAP.h>
#include <DNSServer.h>
#include <ESPmDNS.h>

#include "wifi.h"
#include "logs.h"

#define MAX_WIFI_CHANNEL 13
#define RSSI_AMP 0.2f
#define RSSI_TRESHOLD (-80)
std::vector<unsigned> AP_CHANNELS = {1, 6, 11};


namespace wifi {

    std::vector<String> scannedNetworks;
    std::vector<WiFiConfigEntry> staPriority;
    WiFiConfigEntry apConfig;
    int apChannel = 1;
    IPAddress apAddress, apGateway, apSubnet;
    bool apEnabled = true;
    bool apHidden = false;
    bool apPeriodicScan = false;
    std::uint64_t periodicTimeout = std::uint64_t(1000)*3600*24*365;
    std::uint64_t lastActivity = 0;
    bool currentConnectionPeriodicTimeout = false;
    String hostname;
    DNSServer dnsServer;
    MDNSResponder mdnsResponder;
    std::uint64_t connectionTime = 0;
    unsigned connectionId = 0;
    const char* const TCP = "tcp";
    const char* const HTTP = "http";
    const char* const HTTPS = "https";

    float occupedChannels[MAX_WIFI_CHANNEL+1] = {0};


    void activity() {
        lastActivity = millis();
    }


    void updateConfiguration(const JsonVariantConst configuration) {
        hostname = configuration["wifi"]["hostname"].as<String>();
        const auto apConfigJson = configuration["wifi"]["access_point"];
        apAddress = str2ip(apConfigJson["address"].as<String>());
        apGateway = str2ip(apConfigJson["gateway"].as<String>());
        apSubnet = str2ip(apConfigJson["subnet"].as<String>());
        apConfig.ssid = apConfigJson["ssid"].as<String>();
        apConfig.password = apConfigJson["password"].as<String>();
        apConfig.hidden = apConfigJson["hidden"].as<bool>();
        apChannel = apConfigJson["channel"].as<int>();
        apEnabled = apConfigJson["enabled"].as<bool>();
        apPeriodicScan = apConfigJson["periodic_scan"].as<bool>();
        periodicTimeout = apConfigJson["periodic_scan_time"].as<std::uint64_t>() * 1000;
        if (periodicTimeout < 120*1000) periodicTimeout = 120*1000;

        const auto staPriorityJson = configuration["wifi"]["sta_priority"];
        unsigned size = staPriorityJson.size();
        staPriority.clear();
        staPriority.reserve(size);
        for (unsigned i=0;i<size;i++) {
            const auto entryJson = staPriorityJson[i];
            WiFiConfigEntry entry;
            entry.ssid = entryJson["ssid"].as<String>();
            entry.password = entryJson["password"].as<String>();
            entry.hidden = entryJson["hidden"].as<bool>();
            entry.periodicScan = entryJson["periodic_scan"].as<bool>();
            staPriority.push_back(entry);
        }

    }


    unsigned long long int getConnectionTime() {
        return connectionTime;
    }


    unsigned getConnectionId() {
        return connectionId;
    }


    void updateConnectionStats() {
        connectionTime = millis();
        connectionId++;
    }


    bool eventConnectionStatus = false;
    int eventConnectionMode = 0;
    void afterConnection(int mode);
    void beforeDisconnection(int mode);


    void afterConnection(int mode) {
        if (eventConnectionStatus) 
            beforeDisconnection(eventConnectionMode); 
        if (mode == WIFI_AP) {
            dnsServer.start();
            logs::logger.println("Configured DNS Server");
        }
        mdnsResponder.begin(hostname);
        mdnsResponder.addService(HTTP, TCP, 80);
        mdnsResponder.addService(HTTPS, TCP, 443);
        logs::logger.println("Configured mDNS");
        updateConnectionStats();
        eventConnectionStatus = true;
        eventConnectionMode = mode;
    }


    void beforeDisconnection(int mode) {
        if (!eventConnectionStatus) return;
        mdnsResponder.end();
        logs::logger.println("Deconfigured mDNS");
        if (mode == WIFI_AP) {
            dnsServer.stop();
            logs::logger.println("Deconfigured DNS Server");
        }
        eventConnectionStatus = false;
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


    const char* wl_status_to_string(wl_status_t status) {
        switch (status) {
            case WL_NO_SHIELD: return "WL_NO_SHIELD";
            case WL_IDLE_STATUS: return "WL_IDLE_STATUS";
            case WL_NO_SSID_AVAIL: return "WL_NO_SSID_AVAIL";
            case WL_SCAN_COMPLETED: return "WL_SCAN_COMPLETED";
            case WL_CONNECTED: return "WL_CONNECTED";
            case WL_CONNECT_FAILED: return "WL_CONNECT_FAILED";
            case WL_CONNECTION_LOST: return "WL_CONNECTION_LOST";
            case WL_DISCONNECTED: return "WL_DISCONNECTED";
            case WL_STOPPED: return "WL_STOPPED";
            default: return "UNKNOWN";
        }
    }


    void scanNetworks() {
        for (int i=0;i<=MAX_WIFI_CHANNEL;i++)
            occupedChannels[i] = 0;
        logs::logger.println("Preparing for scanning networks");
        logs::logger.print("Current wifi status: ");
        logs::logger.println(wl_status_to_string(WiFi.status()));
        if (isAP() || WiFi.status() != WL_CONNECTED) disconnect();
        WiFi.mode(WIFI_STA);
        vTaskDelay(100 / portTICK_PERIOD_MS);
        logs::logger.println("Scanning networks");
        int n = WiFi.scanNetworks(false, false, false, 1499);
        logs::logger.print("Scan completed, found networks: ");
        logs::logger.println(n);
        scannedNetworks.clear();
        scannedNetworks.reserve(n);
        logs::logger.println("Reading list....");
        for (int i=0;i<n;i++) {
            scannedNetworks.push_back(WiFi.SSID(i));
            logs::logger.print(" Found network: ");
            logs::logger.println(WiFi.SSID(i));
            unsigned  channel = WiFi.channel(i);
            if (channel <= MAX_WIFI_CHANNEL) 
                occupedChannels[channel] += 1.f / (1.f + exp(RSSI_AMP * (WiFi.RSSI(i) - RSSI_TRESHOLD)));
        }
        WiFi.scanDelete();
    }


    unsigned bestApChannel() {
        float minValue = 0;
        unsigned minChannel = AP_CHANNELS[0];
        for (auto channel : AP_CHANNELS)
            if (occupedChannels[channel] < minValue) {
                minValue = occupedChannels[channel];
                minChannel = channel;
            }
        return minChannel;
    }


    void disconnect() {
        if (WiFi.getMode() == WIFI_STA) {
            beforeDisconnection(WIFI_STA);
            WiFi.disconnect(true, true, 100);
        } else if (WiFi.getMode() == WIFI_AP) {
            beforeDisconnection(WIFI_AP);
            while (!WiFi.softAPdisconnect(true))
                vTaskDelay(10 / portTICK_PERIOD_MS);
        }
        vTaskDelay(100 / portTICK_PERIOD_MS);
    }


    bool connectToNetwork(const WiFiConfigEntry& entry) {
        disconnect();
        WiFi.mode(WIFI_STA);
        vTaskDelay(100 / portTICK_PERIOD_MS);
        logs::logger.print("Connecting to ");
        logs::logger.println(entry.ssid);
        std::uint64_t timeout_time = millis() + CONNECTION_TIMEOUT;
        WiFi.setHostname(hostname.c_str());
        WiFi.begin(entry.ssid, entry.password);
        while (WiFi.status() != WL_CONNECTED && WiFi.status() != WL_CONNECT_FAILED && millis() <= timeout_time)
            vTaskDelay(10 / portTICK_PERIOD_MS);
        bool result = WiFi.status() == WL_CONNECTED;
        if (result) {
            activity();
            currentConnectionPeriodicTimeout = entry.periodicScan;
            logs::logger.print(" -> Connecting success, IP: ");
            logs::logger.println(getLocalIp());
            afterConnection(WIFI_STA);
        } else 
            logs::logger.println(" -> Connecting failed");
        return result;
    }


    void openAccessPoint() {
        logs::logger.println("Switching to AP-mode");
        disconnect();
        activity();
        currentConnectionPeriodicTimeout = apPeriodicScan;
        do {
            WiFi.mode(WIFI_AP);
            vTaskDelay(200 / portTICK_PERIOD_MS);
            WiFi.softAPsetHostname(hostname.c_str());
            WiFi.softAPConfig(apAddress, apGateway, apSubnet);
        } while (!WiFi.softAP(apConfig.ssid, apConfig.password, apChannel ? apChannel : bestApChannel(), apConfig.hidden, 4));        
        activity();
        afterConnection(WIFI_AP);
    }


    bool networkIsInScanned(String name) {
        int size = scannedNetworks.size();
        for (int i=0;i<size;i++)
            if (scannedNetworks[i] == name)
                return true;
        return false;
    }


    void fastInit() {
        if (WiFi.status() == WL_CONNECTED) {
            WiFi.mode(WIFI_STA);
            WiFi.disconnect();
        }

        WiFi.config(INADDR_NONE, INADDR_NONE, INADDR_NONE, INADDR_NONE);
        WiFi.setHostname(hostname.c_str());
    }
    

    bool autoSta() {
        currentConnectionPeriodicTimeout = false;
        for (int i=0;i<staPriority.size();i++) {
            const auto& entry = staPriority[i];
            if (entry.hidden || networkIsInScanned(entry.ssid)) {
                if (connectToNetwork(entry))
                    return true;
            }
        }
        return false;
    }


    void autoConnectWifi() {
        if (staPriority.size() == 0 && apEnabled == false)
            return;
        scanNetworks();
        if (autoSta()) return;
        if (apEnabled) openAccessPoint();
    }


    bool connected() {
        return WiFi.status() == WL_CONNECTED || WiFi.getMode() == WIFI_AP;
    }


    bool isAP() {
        return WiFi.getMode() == WIFI_AP;
    }


    String ip2str(const IPAddress& addr) {
        String dot = ".";
        return String((int)addr[0]) + dot + String((int)addr[1]) + dot + String((int)addr[2]) + dot + String((int)addr[3]);
    }


    String getApAddress() {
        return ip2str(apAddress);
    }


    String getLocalIp() {
        return isAP() ? getApAddress() : ip2str(WiFi.localIP());
    }


    String getHostname() {
        return hostname;
    }


    void checkConnection() {
        if (!connected() || (currentConnectionPeriodicTimeout && millis() - lastActivity > periodicTimeout)) {
            activity();
            autoConnectWifi(); 
        }
    }

}
