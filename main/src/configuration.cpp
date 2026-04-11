#include "configuration.h"

#include <FS.h>
#include <SPIFFS.h>

#include "lib/littlefs/LittleFS.h"
#include "memory_stream.h"
#include "validate_json.h"
#include "hardware_configuration.h"
#include "logs.h"


namespace configuration {

    JsonDocument getResourceJson(const Resource& resource, unsigned size) {
        JsonDocument document;
        deserializeJson(document, resource.data);
        return document;
    }


    JsonDocument getDefautltConfiguration() {
        return getResourceJson(resource_default_config_json);
    }


    JsonDocument getDefautltFavorites() {
        return getResourceJson(resource_default_favorites_json);
    }


    JsonDocument getDefautltAnimations() {
        return getResourceJson(resource_default_animations_json);
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
        versionInfo["hardware"] = hardware::configuration->getCode();
        versionInfo["resources_sha"] = RESOURCES_SHA1;
        return versionInfo;
    }


    void removeFile(const String& filename) {
        LittleFS.remove(filename);
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


    unsigned getFileBinLITTLEFS(const String& filename, std::vector<unsigned char>& buf) {
        File file = LittleFS.open(filename);
        if (file.available()) {
            buf.resize(file.size());
            file.readBytes((char*)buf.data(), file.size());
            file.close();
            return buf.size();
        }
        return 0;
    }


    unsigned getFileBinSPIFFS(const String& filename, std::vector<unsigned char>& buf) {
        File file = SPIFFS.open(filename);
        if (file.available()) {
            buf.resize(file.size());
            file.readBytes((char*)buf.data(), file.size());
            file.close();
            return buf.size();
        }
        return 0;
    }


    unsigned getFileBin(const String& filename, unsigned char* buf, unsigned max_size) {
        File file = LittleFS.open(filename);
        if (file.available()) {
            unsigned result = file.readBytes((char*)buf, std::min(max_size, file.size()));
            file.close();
            return result;
        }
        return 0;
    }


    void serializeToFile(const String filename, const JsonDocument& data) {        
        File file = LittleFS.open(filename, FILE_WRITE);
        serializeJson(data, file);
        file.close();
    }


    void saveFile(const String& filename, const unsigned char* content, unsigned length) {
        File file = LittleFS.open(filename, FILE_WRITE);
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

    
    std::unique_ptr<Stream> getReadStream(const String& filename, const Resource* default_resource) {
        std::unique_ptr<Stream> file = std::make_unique<File>(LittleFS.open(filename, FILE_READ));
        if (file->available()) 
            return file;
        if (default_resource)
            return std::make_unique<MemoryStream>(default_resource->data, default_resource->size);
        return NULL;
    }


    std::unique_ptr<Stream> getConfigStream() {
        return getReadStream(CONFIGURATION_FILENAME, &resource_default_config_json);
    }


    JsonDocument getConfiguration() {
        JsonDocument configuration;
        auto stream = getConfigStream();
        DeserializationError err = deserializeJson(configuration, *stream);
        if (err != DeserializationError::Ok || assertConfiguration(configuration).length())
            configuration = getDefautltConfiguration();
        return configuration;
    }


    std::unique_ptr<Stream> getFavoritesStream() {
        return getReadStream(FAVORITES_FILENAME, &resource_default_favorites_json);
    }


    JsonDocument getFavorites() {
        JsonDocument favorites;
        auto stream = getFavoritesStream();
        DeserializationError err = deserializeJson(favorites, *stream);
        if (err != DeserializationError::Ok || assertJson(favorites, "favorites-list").length())
            favorites = getDefautltFavorites();
        return favorites;
    }


    std::unique_ptr<Stream> getAnimationsStream() {
        return getReadStream(ANIMATIONS_FILENAME, &resource_default_animations_json);
    }


    JsonDocument getAnimations() {
        JsonDocument animations;
        auto stream = getAnimationsStream();
        DeserializationError err = deserializeJson(animations, *stream);
        if (err != DeserializationError::Ok || assertJson(animations, "animations-list").length())
            animations = getDefautltAnimations();
        return animations;
    }


    void deleteCert() {
        removeFile(CERT_KEY_FILE_NAME);
        removeFile(CERT_PUB_FILE_NAME);
    }
    

    void rewriteFilesystem(bool fromSPIFFS=false) {
        logs::logger.println("Rewritting filesystem.");
        const auto convertFunction = fromSPIFFS ? getFileBinSPIFFS : getFileBinLITTLEFS;
        std::vector<unsigned char> configuration;
        std::vector<unsigned char> favorites;
        std::vector<unsigned char> animations;
        std::vector<unsigned char> cert_key;
        std::vector<unsigned char> cert_pub;
        delay(20);
        convertFunction(CONFIGURATION_FILENAME, configuration);
        delay(20);
        convertFunction(FAVORITES_FILENAME, favorites);
        delay(20);
        convertFunction(ANIMATIONS_FILENAME, animations);
        delay(20);
        convertFunction(CERT_KEY_FILE_NAME, cert_key);
        delay(20);
        convertFunction(CERT_PUB_FILE_NAME, cert_pub);
        delay(20);
        
        if (fromSPIFFS) {
            SPIFFS.end();
        } else {
            LittleFS.end();
        }
        LittleFS.begin(true);
        LittleFS.format();

        delay(20);
        if (configuration.size()) saveFile(CONFIGURATION_FILENAME, configuration.data(), configuration.size());
        delay(20);
        if (favorites.size()) saveFile(FAVORITES_FILENAME, favorites.data(), favorites.size());
        delay(20);
        if (animations.size()) saveFile(ANIMATIONS_FILENAME, animations.data(), animations.size());
        delay(20);
        if (cert_key.size()) saveFile(CERT_KEY_FILE_NAME, cert_key.data(), cert_key.size());
        delay(20);
        if (cert_pub.size()) saveFile(CERT_PUB_FILE_NAME, cert_pub.data(), cert_pub.size());        
        delay(20);     
    }


    void mountFileSystem() {
        bool success_read = false;
        bool many_trials = false;
        for (int i=0;i<16;i++) {
	        if (LittleFS.begin(false)) {
                success_read = true;
                break;
            }
            logs::logger.print("Attemption to open file system as LittleFS failed (");
            logs::logger.print(i+1);
            logs::logger.println("/16)");
            many_trials = true;
            delay(100);
        }
        if (!success_read && SPIFFS.begin(false)) {
            logs::logger.print("Attemption to open file system as SPIFFS succeeded.");
            rewriteFilesystem(true);
            logs::logger.print("Rewritted filesystem to LittleFS.");
        } else if (!success_read) {
            LittleFS.begin(true);
        } else if (many_trials) {
            rewriteFilesystem();
        }
    }


    void loadConfiguration() {
        JsonDocument config = getConfiguration();
        if (assertConfiguration(config).length())
            setConfiguration(getDefautltConfiguration());
        else
            setConfiguration(config);
    }


    void init() {
        mountFileSystem();
        loadConfiguration();
    }


    void resetConfiguration() {
        LittleFS.format();
    }


    void setConfiguration(JsonDocument configuration) {
        serializeToFile(CONFIGURATION_FILENAME, configuration);
    }


    void setFavorites(JsonDocument favorites) {
        serializeToFile(FAVORITES_FILENAME, favorites);
    }


    void setAnimations(JsonDocument animations) {
        serializeToFile(ANIMATIONS_FILENAME, animations);
    }


}
