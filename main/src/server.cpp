#include "server.h"

#include <sstream>

#include "fastlz.h"
#include "resources.h"
#include "configuration.h"


namespace server {

    httpsserver::SSLCert cert;
    httpsserver::HTTPSServer* secureServer;
    httpsserver::HTTPServer* insecureServer;


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
        httpsserver::createSelfSignedCert(
            cert,
            httpsserver::KEYSIZE_2048,
            "CN=leddriver.local,O=PawelBielecki,C=PL",
            getCertValidFromDate().c_str(),
            getCertValidUntilDate().c_str()
        );
        configuration::saveFile(CERT_KEY_FILE_NAME, cert.getPKData(), cert.getPKLength());
        configuration::saveFile(CERT_PUB_FILE_NAME, cert.getCertData(), cert.getCertLength());
    }


    void loadCert() {
        auto key = configuration::getFileBin(CERT_KEY_FILE_NAME);
        auto pub = configuration::getFileBin(CERT_PUB_FILE_NAME);
        if (key.size() & pub.size()) {
            cert = httpsserver::SSLCert(
                pub.data(), pub.size(),
                key.data(), pub.size()
            );
        } else 
            generateCert();
    }


    void configure() {
        loadCert();

        secureServer = new httpsserver::HTTPSServer(&cert);
        insecureServer = new httpsserver::HTTPServer();

        ResourceNode* staticNode  = new ResourceNode("", "GET", &sendResource);
        secureServer->setDefaultNode(staticNode);
        insecureServer->setDefaultNode(staticNode);
    }


    void start() {
        secureServer->start();
        insecureServer->start();
    }


    void loop() {
        secureServer->loop();
        insecureServer->loop();
    }


    void addCallback(const char* address, const char* method, const CallbackFunction* callback) {
        ResourceNode* node = new ResourceNode(address, method, callback);
        secureServer->registerNode(node);
        insecureServer->registerNode(node);
    }


    void sendJson(HTTPResponse* res, const JsonVariantConst& data, unsigned bufSize, int costatusCode) {
        char* buf = new char[bufSize];
        unsigned int size = serializeJson(data, buf, bufSize-1);
        buf[size] = 0;
        res->setHeader("Cache-Control", "no-cache");
        res->setHeader("Content-Type", "application/json");
        res->setStatusCode(costatusCode);
        res->print(buf);
        delete [] buf;
    }


    void sendError(HTTPResponse* res, String message, int code = 400) {
        StaticJsonDocument<1024> messageData;
        messageData["status"] = "error";
        messageData["message"] = message;
        char buf[1024];
        unsigned int size = serializeJson(messageData, buf, 1023);
        sendJson(res, messageData, 1024, code);
    }


    void sendOk(HTTPResponse* res) {
        res->setHeader("Cache-Control", "no-cache");
        res->setHeader("Content-Type", "application/json");
        res->setStatusCode(200);
        res->print("{\"status\": \"ok\"}");
    }


    void sendCacheControlHeader(HTTPResponse* res) {
        int minAge = 432000; 
        int maxAge = 604800; 
        int randomAge = random(minAge, maxAge + 1);
        String headerValue = "max-age=" + String(randomAge);
        res->setHeader("Cache-Control", headerValue.c_str());
    }


    int sendDecompressedData(HTTPResponse* res, const Resource& resource, int statusCode) {
		uint8_t* decompressed_buffer = new uint8_t[resource.decompressed_size];
		size_t decompressed_size = fastlz_decompress(resource.data, resource.size, decompressed_buffer, resource.decompressed_size);
		if (decompressed_size == 0) {
				sendError(res, "Decompression error");
				delete [] decompressed_buffer;
				return 0;
		}
		sendCacheControlHeader(res);
        res->setHeader("Content-Type", resource.mime_type);
        res->setStatusCode(statusCode);
		res->write(decompressed_buffer, decompressed_size);
		delete [] decompressed_buffer;
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


    void sendResource(HTTPRequest* req, HTTPResponse* res) {
        const Resource& resource = getResourceByName(req->getRequestString().c_str(), &resource_not_found_html);
        sendDecompressedData(res, resource, resource.name == resource_not_found_html.name ? 404 : 200); //yes, we can compare const char* in this context
    }


    std::vector<char> readBuffer(HTTPRequest* req) {
        std::vector<char> result;
        result.resize(MAX_POST_SIZE);
        unsigned size = 0;
        while (!req->requestComplete() && size < MAX_POST_SIZE) {
            size += req->readChars(result.data() + size, MAX_POST_SIZE-size);
        }
        result.resize(size);
        result.shrink_to_fit();
        return std::move(result);
    }


    bool readJson(HTTPRequest* req, HTTPResponse* res, JsonDocument& json, const String& assertEntryName) {
        std::vector<char> rawData = server::readBuffer(req);
        DeserializationError err = deserializeJson(json, (const char*)rawData.data());
        if (sendDeserializationError(res, err)) 
            return false;
        if (assertEntryName != "") {
            const String assertMessage = configuration::assertJson(json, assertEntryName);
            if (assertMessage != "") {
                sendError(res, assertMessage, 422);
                return false;
            }
        }
        return true;
    }

}
