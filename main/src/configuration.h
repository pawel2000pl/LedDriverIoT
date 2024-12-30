#pragma once

#include <Arduino.h>
#include <ArduinoJson.h>
#include <vector>
#include "resources.h"

#define JSON_CONFIG_BUF_SIZE (16*1024)
#define JSON_FAVORITES_BUF_SIZE (64*64)
#define JSON_VERSION_INFO_BUF_SIZE (1024)
#define CONFIGURATION_FILENAME "/configuration.json"
#define FAVORITES_FILENAME "/favorites.json"
#define CERT_KEY_FILE_NAME "/cert.key"
#define CERT_PUB_FILE_NAME "/cert.pub"


namespace configuration {

    void init();
    void removeFile(const String& filename);
    std::vector<unsigned char>* getFileBin(const String& filename);
    String getFileStr(const String& filename);
    unsigned checkSum(const unsigned char* ptr, unsigned size, unsigned d=1617849293);
    void saveFile(const String& filename, const unsigned char* content, unsigned length);
    void saveFile(const String& filename, const char* content);
    void saveFile(const String& filename, const String& content);
    String getResourceStr(const struct Resource& resource);
    JsonDocument getResourceJson(const struct Resource& resource, unsigned size=0);
    JsonDocument getDefautltConfiguration();
    JsonDocument getDefautltFavorites();
    JsonDocument getConfigSchema();
    String assertJson(JsonVariant configuration, String name);
    String assertConfiguration(JsonVariant configuration);
    JsonDocument getVersionInfo();
    String getConfigurationStr();
    JsonDocument getConfiguration();
    JsonDocument getFavorites();
    void resetConfiguration();
    void setConfiguration(JsonDocument configuration);
    void setFavorites(JsonDocument favorites);
    void deleteCert();

}
