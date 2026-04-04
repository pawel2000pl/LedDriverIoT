#include "server.h"

#include <sstream>
#include <vector>
#include <esp_system.h>
#include <esp_random.h>
	
#include "logs.h"
#include "wifi.h"
#include "constrain.h"
#include "resources.h"
#include "configuration.h"
#include "inplace_vector.h"
#include "dynamic_buffers.h"


#define MAX_POST_SIZE (16*1024-1)


namespace server {

    const unsigned max_cert_size = 2048;
    unsigned cert_pub_size = 0;
    unsigned cert_key_size = 0;
    uint8_t cert_pub_data[max_cert_size] = {0};
    uint8_t cert_key_data[max_cert_size] = {0};
    httpsserver::SSLCert cert;
    httpsserver::HTTPSServer secureServer(&cert, 443, 1);
    httpsserver::HTTPServer insecureServer(80, 2);
    inplace_vector<ResourceNode, 48> resourceNodes;
    bool captivePortalEnabled = false;
    unsigned queryId = 0;
    bool queryFlag = false;


    unsigned getQueryId() {
        return queryId;
    }


    bool resetQueryFlag() {
        bool flag = queryFlag;
        queryFlag = false;
        return flag;
    }


    void updateConfiguration(const JsonVariantConst configuration) {
        captivePortalEnabled = configuration["wifi"]["access_point"]["captive"].as<bool>();
    }


    void extractMacroDate(int &day, int &month, int &year) {
        std::string dateStr = __DATE__;
        std::istringstream iss(dateStr);
        std::string monthStr;
        std::string dayStr;
        
        iss >> monthStr >> dayStr >> year;

        if (monthStr == "Jan") month = 1;
        else if (monthStr == "Feb") month = 2;
        else if (monthStr == "Mar") month = 3;
        else if (monthStr == "Apr") month = 4;
        else if (monthStr == "May") month = 5;
        else if (monthStr == "Jun") month = 6;
        else if (monthStr == "Jul") month = 7;
        else if (monthStr == "Aug") month = 8;
        else if (monthStr == "Sep") month = 9;
        else if (monthStr == "Oct") month = 10;
        else if (monthStr == "Nov") month = 11;
        else if (monthStr == "Dec") month = 12;

        day = std::stoi(dayStr);
    }


    std::string buildDate(int day, int month, int year) {
        char buf[16];
        int len = sprintf(buf, "%04d%02d%02d000000", year, month, day);
        buf[len] = 0;
        return std::string(buf);
    }


    std::string getCertValidFromDate() {
        int day, month, year;
        extractMacroDate(day, month, year);
        return buildDate(day, month, year);
    }


    std::string getCertValidUntilDate() {
        int day, month, year;
        extractMacroDate(day, month, year);
        return buildDate(day, month, year+250);
    }


    void generateCert() {
        httpsserver::SSLCert* newCert = new httpsserver::SSLCert();
        std::string fromDate = getCertValidFromDate();
        std::string untilDate = getCertValidUntilDate();
        httpsserver::createSelfSignedCert(
            *newCert,
            httpsserver::KEYSIZE_1024,
            "CN=leddriver.local,O=PawelBielecki,C=PL",
            fromDate.c_str(),
            untilDate.c_str()
        );
        cert_pub_size = newCert->getCertLength();
        cert_key_size = newCert->getPKLength();
        memcpy(cert_pub_data, newCert->getCertData(), cert_pub_size);
        memcpy(cert_key_data, newCert->getPKData(), cert_key_size);
        configuration::saveFile(CERT_PUB_FILE_NAME, cert_pub_data, cert_pub_size);
        configuration::saveFile(CERT_KEY_FILE_NAME, cert_key_data, cert_key_size);
        newCert->clear();
        delete newCert;
        cert.setCert(cert_pub_data, cert_pub_size);
        cert.setPK(cert_key_data, cert_key_size);
    }


    void loadCert() {
        cert_pub_size = configuration::getFileBin(CERT_PUB_FILE_NAME, cert_pub_data, max_cert_size);
        cert_key_size = configuration::getFileBin(CERT_KEY_FILE_NAME, cert_key_data, max_cert_size);
        if (cert_pub_size && cert_key_size) {
            cert.setCert(cert_pub_data, cert_pub_size);
            cert.setPK(cert_key_data, cert_key_size);
        } else {
            generateCert();
        }
    }


    void middleware(HTTPRequest*, HTTPResponse*, std::function<void()> next) {
        wifi::activity();
        queryId++;
        queryFlag = true;
        next();
        wifi::activity();
    }


    void configure() {
        loadCert();

        for (int i=0;i<resourceNodes.size();i++) {
            secureServer.unregisterNode(&resourceNodes[i]);
            insecureServer.unregisterNode(&resourceNodes[i]);
        }
        resourceNodes.clear();
        secureServer.removeMiddleware(middleware);
        insecureServer.removeMiddleware(middleware);

        insecureServer.addMiddleware(middleware);
        secureServer.addMiddleware(middleware);
        ResourceNode* staticNode = resourceNodes.emplace_back("", "GET", &sendResource);
        insecureServer.setDefaultNode(staticNode);
        secureServer.setDefaultNode(staticNode);

        // test start
        secureServer.start();
        if (secureServer.isRunning())             
            secureServer.stop();
        else {
            configuration::removeFile(CERT_PUB_FILE_NAME);
            configuration::removeFile(CERT_KEY_FILE_NAME);
        }
    }


    bool serverStarted = false;

    void start() {
        if (serverStarted) return;
        insecureServer.start();
        vTaskDelay(10 / portTICK_PERIOD_MS);
        secureServer.start();
        vTaskDelay(10 / portTICK_PERIOD_MS);
        serverStarted = true;
        logs::logger.println(insecureServer.isRunning() ? "inssecure server ok" : "inssecure server err");
        logs::logger.println(secureServer.isRunning() ? "secure server ok" : "secure server err");
    }


    void stop() {
        if (!serverStarted) return;
        secureServer.stop();
        insecureServer.stop();
        serverStarted = false;
    }


    void restart() {
        stop();
        start();
    }


    void loop() {
        if (!serverStarted) return;
        unsigned freeBlock = heap_caps_get_largest_free_block(MALLOC_CAP_DEFAULT);
        if (freeBlock >= 4 * 1024)
            insecureServer.loop();
        if (freeBlock >= 32 * 1024)
            secureServer.loop();
    }


    void addCallback(const char* address, const char* method, const CallbackFunction* callback) {
        ResourceNode* node = resourceNodes.emplace_back(address, method, callback);
        insecureServer.registerNode(node);
        secureServer.registerNode(node);
    }


    bool sendJsonStatic(HTTPResponse* res, const JsonVariantConst data, unsigned bufSize) {
        std::vector<char> buf(bufSize);
        unsigned int size = serializeJson(data, buf.data(), bufSize);
        if (size >= bufSize) return false;
        buf[size] = 0;
        char size_str[24];
        res->setHeader("Content-Length", itoa(size, size_str, 10));
        uint8_t* buf_data = (uint8_t*)buf.data();
        size_t sent = 0;
        while (sent < size) {
            size_t send_size = std::min<size_t>(256, size - sent);
		    res->write(buf_data + sent, send_size);
            sent += send_size;
        }
        return true;
    }


    void sendJsonDynamic(HTTPResponse* res, const JsonVariantConst data) {
        ResponseWriter writer(res);
        serializeJson(data, writer);
    }


    void sendJson(HTTPResponse* res, const JsonVariantConst data, unsigned bufSize, int statusCode) {
        unsigned freeBlock = heap_caps_get_largest_free_block(MALLOC_CAP_DEFAULT);
        res->setHeader("Cache-Control", "no-cache");
        res->setHeader("Content-Type", "application/json");
        res->setStatusCode(statusCode);
        if ((bufSize==0) || (4 * bufSize >= freeBlock) || !sendJsonStatic(res, data, bufSize)) 
            sendJsonDynamic(res, data);
    }


    void sendError(HTTPResponse* res, String message, int code = 400) {
        JsonDocument messageData;
        messageData["status"] = "error";
        messageData["message"] = message;
        sendJson(res, messageData, 1024, code);
    }


    void sendOk(HTTPResponse* res) {
        const char* buf = "{\"status\": \"ok\"}";
        char size_str[24];
        int size = strlen(buf);
        res->setHeader("Cache-Control", "no-cache");
        res->setHeader("Content-Type", "application/json");
        res->setHeader("Content-Length", itoa(size, size_str, 10));
        res->setStatusCode(200);
        res->write((uint8_t*)buf, size);
    }


    void sendCacheControlHeader(HTTPResponse* res) {
        int minAge = 432000; 
        int maxAge = 604800; 
        int randomAge = minAge + (esp_random() % (maxAge - minAge));
        String headerValue = "max-age=" + String(randomAge);
        res->setHeader("Cache-Control", headerValue.c_str());
    }


    int sendResourceData(HTTPResponse* res, const Resource& resource, int statusCode) {
		if (statusCode >= 200 && statusCode < 300) 
            sendCacheControlHeader(res);
        char size_str[24];
        res->setHeader("Content-Type", resource.mime_type);
        res->setHeader("Content-Length", itoa(resource.size, size_str, 10));
        res->setHeader("ETag", resource.etag);
        res->setStatusCode(statusCode);
        size_t sent = 0;
        while (sent < resource.size) {
            size_t send_size = std::min<size_t>(256, resource.size - sent);
		    res->write((uint8_t*)(resource.data+sent), send_size);
            sent += send_size;
        }
		return 1;
    }


    bool sendDeserializationError(HTTPResponse* res, DeserializationError err) {
        if (err == DeserializationError::Ok) return false;
        else if (err == DeserializationError::EmptyInput) sendError(res, "Empty input", 400);
        else if (err == DeserializationError::IncompleteInput) sendError(res, "Incomplete input", 422);
        else if (err == DeserializationError::InvalidInput) sendError(res, "Invalid input", 422);
        else if (err == DeserializationError::NoMemory) sendError(res, "Out of memory (content too long)", 413);
        else if (err == DeserializationError::TooDeep) sendError(res, "Data structure is too deep", 413);
        else sendError(res, "Unknown error in deserialization", 400);
        return true;
    }


    bool strContains(const std::string& s, const String& sub) {
        return s.find(sub.c_str()) != std::string::npos;
    }


    bool refererIsMe(HTTPRequest* req) {
        std::string referer = req->getHeader("Referer");
        std::string host = req->getHeader("Host");
        String ip = wifi::getLocalIp();
        String hostname = wifi::getHostname() + ".local";
        return strContains(referer, ip) 
            || strContains(referer, hostname) 
            || strContains(host, ip) 
            || strContains(host, hostname);
    }


    void sendResource(HTTPRequest* req, HTTPResponse* res) {
        const Resource& resource = getResourceByName(req->getRequestString().c_str(), &resource_not_found_html);
        bool notFound = resource.name == resource_not_found_html.name; //yes, we can compare const char* in this context
        bool useCaptive = captivePortalEnabled && wifi::isAP() && !refererIsMe(req);
        if (notFound && useCaptive) {
            String apAddress = wifi::getApAddress();
            res->setHeader("Location", "http://" + std::string(apAddress.c_str()) + "/");
        }
        if ((!notFound) && (req->getHeader("If-None-Match").find(resource.etag) != std::string::npos)) {
            sendCacheControlHeader(res);
            res->setHeader("ETag", resource.etag);
            res->setStatusCode(304);
            return;
        }
        sendResourceData(res, resource, notFound ? (useCaptive ? 302 : 404) : 200); 
    }


    unsigned getContentLength(HTTPRequest* req, unsigned defaultValue) {
        std::string content_length = req->getHeader("Content-Length");
        if (!content_length.length()) return defaultValue;
        return atoi(content_length.c_str());
    }


    void readBuffer(HTTPRequest* req, bool addZero, std::vector<char>& result) {
        std::string content_length = req->getHeader("Content-Length");
        int buf_size = constrain(atoi(content_length.c_str()), 0, MAX_POST_SIZE);
        if (!buf_size) buf_size = MAX_POST_SIZE;
        result.resize(buf_size + !!addZero);
        char* result_data = result.data();
        unsigned size = 0;
        while (!req->requestComplete() && size < buf_size) {
            size += req->readChars(result_data + size, buf_size-size);
        }
        if (addZero && (size == 0 || result_data[size-1] != 0)) 
            result_data[size++] = 0;
        result.resize(size);
    }


    bool readJson(HTTPRequest* req, HTTPResponse* res, JsonDocument& json, const String& assertEntryName) {
        unsigned freeHeap = esp_get_free_heap_size() - (assertEntryName.length() ? resource_config_schema_json.size * 2 : 0);
        unsigned maximumContentLength = min(freeHeap / 2, (unsigned)MAX_POST_SIZE);
        unsigned contentLength = getContentLength(req, maximumContentLength);
        if (contentLength > maximumContentLength) {
            server::sendError(res, "Content too large", 413);
            return false;
        }
        DeserializationError err = DeserializationError::Ok;
        if (4 * contentLength < freeHeap) {
            std::vector<char> rawData;
            server::readBuffer(req, true, rawData);
            err = deserializeJson(json, (const char*)(rawData.data()));
        } else {
            RequestReader reader = RequestReader(req);
            err = deserializeJson(json, reader);
        }
        if (sendDeserializationError(res, err)) 
            return false;
        if (assertEntryName.length()) {
            const String assertMessage = configuration::assertJson(json, assertEntryName);
            if (assertMessage.length()) {
                sendError(res, assertMessage, 422);
                return false;
            }
        }
        return true;
    }

}
