#pragma once

#include <Arduino.h>
#include <vector>
#include "lib/ArduinoJson/ArduinoJson.h"
#include "resources.h"
#include "json_utils.h"
#include "validate_json.h"
#include "memory"

#define CONFIGURATION_FILENAME "/configuration.json"
#define FAVORITES_FILENAME "/favorites.json"
#define ANIMATIONS_FILENAME "/animations.json"
#define CERT_KEY_FILE_NAME "/cert.key"
#define CERT_PUB_FILE_NAME "/cert.pub"


namespace configuration {

    void init();
    void removeFile(const String& filename);
    unsigned getFileBin(const String& filename, unsigned char* buf, unsigned max_size);
    unsigned checkSum(const unsigned char* ptr, unsigned size, unsigned d=1617849293);
    bool saveFile(const String& filename, const unsigned char* content, unsigned length);
    bool saveFile(const String& filename, const char* content);
    bool saveFile(const String& filename, const String& content);
    JsonDocument getResourceJson(const struct Resource& resource, unsigned size=0);
    JsonDocument getDefautltConfiguration();
    JsonDocument getDefautltFavorites();
    JsonDocument getDefautltAnimations();
    JsonDocument getConfigSchema();
    String assertJson(JsonVariant configuration, String name, const JsonVariantConst defaults=JsonNull);
    String assertConfiguration(JsonVariant configuration);
    JsonDocument getVersionInfo();

    std::unique_ptr<Stream> getReadStream(const String& filename, const Resource* default_resource);
    std::unique_ptr<Stream> getConfigStream();
    std::unique_ptr<Stream> getAnimationsStream();

    JsonDocument getConfiguration();
    JsonDocument getFavorites();
    JsonDocument getAnimations();
    void resetConfiguration();
    void setConfiguration(JsonDocument configuration);
    void setFavorites(JsonDocument favorites);
    void setAnimations(JsonDocument animations);
    void deleteCert();

}
