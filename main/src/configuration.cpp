#include "configuration.h"

#include <FS.h>
#include <SPIFFS.h>

#include "fastlz.h"
#include "validate_json.h"
#include "hardware_configuration.h"


namespace configuration {


    String getResourceStr(const Resource& resource) {
        char decompressed_buffer [resource.decompressed_size+1];
        size_t decompressed_size = fastlz_decompress(resource.data, resource.size, decompressed_buffer, resource.decompressed_size);
        decompressed_buffer[decompressed_size] = 0;
        return String(decompressed_buffer);
    }


    JsonDocument getResourceJson(const Resource& resource, unsigned size) {
        JsonDocument document;
        deserializeJson(document, getResourceStr(resource));
        return document;
    }


    JsonDocument getDefautltConfiguration() {
        return getResourceJson(resource_default_config_json);
    }


    JsonDocument getDefautltFavorites() {
        return getResourceJson(resource_default_favorites_json);
    }


    JsonDocument getConfigSchema() {
        return getResourceJson(resource_config_schema_json);
    }


    String assertJson(JsonVariant configuration, String name) {
        const auto configSchema = getConfigSchema();
        return validateJson(configuration, configSchema, configSchema[name], ".", getDefautltConfiguration()); 
    }


    String assertConfiguration(JsonVariant configuration) {
        return assertJson(configuration, "main");
    }


    JsonDocument getVersionInfo() {
        auto versionInfo = getResourceJson(resource_version_json, 1024);
        versionInfo["date"] = __DATE__;
        versionInfo["time"] = __TIME__;
        versionInfo["hardware"] = hardware_configuration.getCode();
        versionInfo["resources_sha"] = RESOURCES_SHA1;
        return versionInfo;
    }


    void removeFile(const String& filename) {
        SPIFFS.remove(filename);
    }


    unsigned checkSum(const unsigned char* ptr, unsigned size, unsigned d) {
        unsigned long long int buf = 1;
        const unsigned char* endloop = ptr + size;
        for (auto i = ptr; i < endloop; i++) {
            buf = ((buf << 8) | *i);
            if (buf >> 56) buf %= d;
        }
        return (buf < d) ? buf % d : buf;
    }


    std::vector<unsigned char> getFileBin(const String& filename) {
        File file = SPIFFS.open(filename);
        if (file) {
            std::vector<unsigned char> result;
            result.resize(file.size());
            file.read(result.data(), file.size());
            file.close();
            return std::move(result);
        }
        return std::vector<unsigned char>();
    }


    String getFileStr(const String& filename) {
        File file = SPIFFS.open(filename);
        if (file) {
            String buf = file.readString();
            file.close();
            return buf;
        }
        return "";
    }


    void saveFile(const String& filename, const unsigned char* content, unsigned length) {
        File file = SPIFFS.open(filename, FILE_WRITE);
        file.write(content, length);
        file.close();
    }


    void saveFile(const String& filename, const char* content) {
        unsigned length = -1;
        while (content[++length]);
        saveFile(filename, (const unsigned char*)content, length);
    }


    void saveFile(const String& filename, const String& content) {
        saveFile(filename, (const unsigned char*)content.c_str(), content.length());
    }


    String getConfigurationStr() {
        String buf = getFileStr(CONFIGURATION_FILENAME);
        return buf == "" ? getResourceStr(resource_default_config_json) : buf;
    }


    JsonDocument getConfiguration() {
        JsonDocument configuration;
        String buf = getConfigurationStr();
        DeserializationError err = deserializeJson(configuration, buf);
        if (err != DeserializationError::Ok || assertConfiguration(configuration).length())
            configuration = getDefautltConfiguration();
        return configuration;
    }


    JsonDocument getFavorites() {
        JsonDocument favorites;
        String buf = getFileStr(FAVORITES_FILENAME);
        DeserializationError err = deserializeJson(favorites, buf);
        if (err != DeserializationError::Ok || assertJson(favorites, "favorites-list").length())
            favorites = getDefautltFavorites();
        return favorites;
    }


    void deleteCert() {
        removeFile(CERT_KEY_FILE_NAME);
        removeFile(CERT_PUB_FILE_NAME);
    }
    

    void init() {
	    SPIFFS.begin(true);
        JsonDocument config = getConfiguration();
        if (assertConfiguration(config).length())
            setConfiguration(getDefautltConfiguration());
        else
            setConfiguration(config);
    }


    void resetConfiguration() {
        SPIFFS.format();
    }


    void setConfiguration(JsonDocument configuration) {
        char* buf = new char[JSON_CONFIG_BUF_SIZE+1];
        unsigned size = serializeJson(configuration, buf, JSON_CONFIG_BUF_SIZE);
        buf[size] = 0;
        saveFile(CONFIGURATION_FILENAME, (unsigned char*)buf, size);
        delete [] buf;
    }


    void setFavorites(JsonDocument favorites) {
        char* buf = new char[JSON_FAVORITES_BUF_SIZE+1];
        unsigned size = serializeJson(favorites, buf, JSON_FAVORITES_BUF_SIZE);
        saveFile(FAVORITES_FILENAME, (unsigned char*)buf, size);
        delete [] buf;
    }


}
