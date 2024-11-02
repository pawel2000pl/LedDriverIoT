#include <Arduino.h>
#include <FS.h>
#include <list>
#include <SPIFFS.h>
#include <Update.h>
#include <functional>
#include <ArduinoJson.h>
#include <driver/temperature_sensor.h>
#include <HTTPRequest.hpp>
#include <HTTPResponse.hpp>
#include <HTTPMultipartBodyParser.hpp>

#include "wifi.h"
#include "knobs.h"
#include "fastlz.h"
#include "inputs.h"
#include "outputs.h"
#include "constrain.h"
#include "resources.h"
#include "ledc_driver.h"
#include "conversions.h"
#include "validate_json.h"
#include "configuration.h"
#include "hardware_configuration.h"
#include "server.h"

#define FAN_TURN_ON_TEMP 70
#define FAN_TURN_OFF_TEMP 50
#define THERMISTOR_CONST 4050.0f
#define THERMISTOR_R0 47000
#define THERMISTOR_IN_SERIES_RESISTOR 47000
#define THERMISTOR_T0 (25.0f + 273.15f)

#define RESET_CONF_TIME 10000
#define RARE_CHECKS_INTERVAL (15000)


std::list<std::function<void()>> taskQueue;
String webColorSpace;
bool whiteKnobEnabled;


void updateModules(JsonVariant configuration) {
	knobs::updateConfiguration(configuration);
	inputs::updateConfiguration(configuration);
	outputs::updateConfiguration(configuration);
	wifi::updateConfiguration(configuration);
	webColorSpace = configuration["channels"]["webMode"].as<String>();
	whiteKnobEnabled = configuration["hardware"]["enbleWhiteKnob"].as<bool>();
}


void updateModules() {
	auto configuration = configuration::getConfiguration();
	updateModules(configuration);
}


void resetConfiguration() {
	configuration::resetConfiguration();
	updateModules();
}


void sendConfiguration(HTTPRequest* req, HTTPResponse* res) {
	String buf = configuration::getConfigurationStr();
	res->setHeader("Cache-Control", "no-cache");
  	res->setHeader("Content-Type", "application/json");
	res->setStatusCode(200);
	res->print(buf.c_str());
}


void customValidator(HTTPRequest* req, HTTPResponse* res) {
	DynamicJsonDocument testJson(JSON_CONFIG_BUF_SIZE);
	if (!server::readJson(req, res, testJson)) return;
	const auto configSchema = configuration::getConfigSchema();
	const String assertMessage = 
		testJson.containsKey("type") ? 
		validateJson(testJson["data"], configSchema, configSchema[testJson["type"].as<String>()]) : 
		validateJson(testJson["data"], testJson["schema"], testJson["schema"]["main"]);
	if (assertMessage != "") {
		server::sendError(res, assertMessage, 422);
		return;
	}
	server::sendOk(res);
}


void connectToNetworkEndpoint(HTTPRequest* req, HTTPResponse* res) {
	StaticJsonDocument<768> data;
	if (!server::readJson(req, res, data, "wifi-entry")) return;
	server::sendOk(res);
	const String ssid = data["ssid"].as<String>();
	const String password = data["password"].as<String>();
	taskQueue.push_back([=](){wifi::connectToNetwork(ssid, password);});
}


void autoScanWifiEndpoint(HTTPRequest* req, HTTPResponse* res) {
	server::sendOk(res); 
	taskQueue.push_back(wifi::scanNetworks);
}


void openAccessPointEndpoint(HTTPRequest* req, HTTPResponse* res) {
	server::sendOk(res); 
	taskQueue.push_back(wifi::openAccessPoint);
}


void recvConfiguration(HTTPRequest* req, HTTPResponse* res) {
	DynamicJsonDocument configuration(JSON_CONFIG_BUF_SIZE);
	if (!server::readJson(req, res, configuration, "main")) return;
	configuration::setConfiguration(configuration);
	updateModules(configuration);
	server::sendOk(res);
}

/*
void update(HTTPRequest* req, HTTPResponse* res) {
	HTTPUpload& upload = server.upload();
	if (upload.status == UPLOAD_FILE_START) {
		Serial.printf("Update: %s\n", upload.filename.c_str());
		if (!Update.begin(UPDATE_SIZE_UNKNOWN)) {
			Update.printError(Serial);
		}
	} else if (upload.status == UPLOAD_FILE_WRITE) {
		if (Update.write(upload.buf, upload.currentSize) != upload.currentSize) {
			Update.printError(Serial);
		}
	} else if (upload.status == UPLOAD_FILE_END) {
		Serial.println("Finished");
		Update.end(true);
		Update.printError(Serial);    
	}
	if (Update.hasError()) 
		sendError(Update.errorString());
	else {
		invalidateCache();
		taskQueue.push_back([](){ESP.restart();});
	}
}
*/


void update(HTTPRequest* req, HTTPResponse* res) {
    // Sprawdzenie typu Content-Type, aby upewnić się, że jest to multipart/form-data
    std::string contentType = req->getHeader("Content-Type");
    size_t semicolonPos = contentType.find(";");
    if (semicolonPos != std::string::npos) {
        contentType = contentType.substr(0, semicolonPos);
    }

    if (contentType != "multipart/form-data") {
        Serial.printf("Unknown POST Content-Type: %s\n", contentType.c_str());
		server::sendError(res, "Unsupported Content-Type", 415);
        return;
    }

    // Tworzenie parsera dla multipart/form-data
    httpsserver::HTTPMultipartBodyParser* parser = new httpsserver::HTTPMultipartBodyParser(req);
    bool updateStarted = false;
    bool updateError = false;

    while (parser->nextField()) {
        // Odczytanie nazwy pola i nazwy pliku
        std::string name = parser->getFieldName();
        std::string filename = parser->getFieldFilename();

        // Jeśli pole nie jest plikiem, przechodzimy dalej
        if (name != "file") {
            Serial.println("Skipping unexpected field");
            continue;
        }

        Serial.printf("Update: %s\n", filename.c_str());

        // Rozpoczęcie procesu aktualizacji
        if (!Update.begin(UPDATE_SIZE_UNKNOWN)) {
            Update.printError(Serial);
			server::sendError(res, "Cannot begin upgrading", 500);
            updateError = true;
            break;
        }
        updateStarted = true;

        // Pobieranie i zapisywanie danych pliku
        while (!parser->endOfField()) {
            byte buf[512];
            size_t readLength = parser->read(buf, sizeof(buf));
            if (Update.write(buf, readLength) != readLength) {
                Update.printError(Serial);
				server::sendError(res, "Error during uploading - probably too large update file", 500);
                updateError = true;
                break;
            }
        }
        if (updateError) break;
    }

    delete parser;

	if (updateError) {
		if (updateStarted) 
			Update.abort();
		return;
	}

    // Zakończenie aktualizacji
    if (updateStarted) {
        if (Update.end(true)) {  // true oznacza, że zakończono z sukcesem
            Serial.println("Update Finished");
			server::sendOk(res);
            invalidateCache(req, res);
            taskQueue.push_back([]() { ESP.restart(); });
        } else {
            Serial.println("Update failed at end");
            Update.printError(Serial);
			server::sendError(res, "Update failed", 500);
        }
    } else {
        Serial.println("No update file received");
		server::sendError(res, "No update file received", 400);
    }
}


void invalidateCache(HTTPRequest* req, HTTPResponse* res) {
	res->setHeader("Clear-Site-Data", "\"cache\"");
	res->setHeader("Connection", "close");
	server::sendOk(res);
}


void getVersionInfo(HTTPRequest* req, HTTPResponse* res) {
	server::sendJson(res, configuration::getVersionInfo());
}


void sendNetworks(HTTPRequest* req, HTTPResponse* res) {
	unsigned lengthSum = 0;
	const auto& scannedNetworks = wifi::getScannedNetworks();
	for (auto s : scannedNetworks) lengthSum += s.length();
	unsigned bufSize = lengthSum + 64 * scannedNetworks.size() + 32;
	DynamicJsonDocument networkList(bufSize);
	for (auto s : scannedNetworks) networkList.add(s);
	server::sendJson(res, networkList, bufSize);
}


void sendFavorites(HTTPRequest* req, HTTPResponse* res) {
	DynamicJsonDocument favorites = configuration::getFavorites();
	String colorspace = webColorSpace;
	DynamicJsonDocument response(JSON_FAVORITES_BUF_SIZE);
	JsonArray reponseArray = response.to<JsonArray>();
	unsigned size = favorites.size();
	for (unsigned i=0;i<size;i++) {
		JsonObject item = reponseArray.createNestedObject();
		String code = favorites[i].as<String>();
		item["code"] = code;
		ColorChannels channels = inputs::favoriteColorPreview(colorspace, code);
		JsonArray colorJson = item.createNestedArray("color");
		colorJson.add(channels[0]);
		colorJson.add(channels[1]);
		colorJson.add(channels[2]);
		colorJson.add(channels[3]);
	}
	server::sendJson(res, response, JSON_FAVORITES_BUF_SIZE);
}


void dumpFavorite(HTTPRequest* req, HTTPResponse* res) {
	std::string white = "0";
	req->getParams()->getQueryParameter("white", white);
	bool useWhite = white != "0";
	String dumped = inputs::dumpFavoriteColor(useWhite);
	ColorChannels channels = inputs::getAuto(webColorSpace);
	char buf[256];
	int size = sprintf(buf, "{\"code\": \"%s\", \"color\": [%f, %f, %f, %f]}", 
		dumped.c_str(),
		channels[0],
		channels[1],
		channels[2],
		channels[3]
	);
	buf[size] = 0;
	res->setHeader("Cache-Control", "no-cache");
	res->setHeader("Content-Type", "application/json");
	res->println(buf);
}


void saveFavorites(HTTPRequest* req, HTTPResponse* res) {
	DynamicJsonDocument data(JSON_FAVORITES_BUF_SIZE);
	if (!server::readJson(req, res, data, "favorites-list")) return;
	configuration::setFavorites(data);
	server::sendOk(res);
}


void applyFavorite(HTTPRequest* req, HTTPResponse* res) {
	std::string code = "000000000";
	req->getParams()->getQueryParameter("code", code);
	knobs::turnOff();
	inputs::applyFavoriteColor(String(code.c_str()));
	outputs::writeOutput();
	server::sendOk(res);
}


void sendColors(HTTPRequest* req, HTTPResponse* res) {
	ColorChannels channels = inputs::getAuto(webColorSpace);
	char buf[256];
	int size = sprintf(buf, "[%f, %f, %f, %f]", 
		channels[0],
		channels[1],
		channels[2],
		channels[3]
	);
	buf[size] = 0;
	res->setHeader("Cache-Control", "no-cache");
	res->setHeader("Content-Type", "application/json");
	res->println(buf);
}


void setColors(HTTPRequest* req, HTTPResponse* res) {
	StaticJsonDocument<256> data;
	if (!server::readJson(req, res, data, "color-channels")) return;
	inputs::setAuto(webColorSpace, {data[0].as<float>(), data[1].as<float>(), data[2].as<float>(), data[3].as<float>()});
	outputs::writeOutput();
	knobs::turnOff();
	server::sendOk(res);   
}


int floatFilter15(float x) { 
	return (int)round(x * 15.f); 
};


void renderFavoriteColor(HTTPRequest* req, HTTPResponse* res) {
	String templateStr = configuration::getResourceStr(resource_favorite_color_template_html);
	std::string code = "000000000";
	req->getParams()->getQueryParameter("code", code);
	inputs::applyFavoriteColor(String(code.c_str()));
	outputs::writeOutput();
	knobs::turnOff();

	const auto& channelsMode = webColorSpace;
	ColorChannels filteredChannels = inputs::getAuto(channelsMode);

	float r, g, b;
	if (channelsMode == "rgb") rgbToRgb(filteredChannels[0], filteredChannels[1], filteredChannels[2], r, g, b);
	if (channelsMode == "hsl") hslToRgb(filteredChannels[0], filteredChannels[1], filteredChannels[2], r, g, b);
	if (channelsMode == "hsv") hsvToRgb(filteredChannels[0], filteredChannels[1], filteredChannels[2], r, g, b);
	char* render_buffer = new char[favorite_color_template_html_decompressed_size+256];
	sprintf(
		render_buffer, templateStr.c_str(),
		(int)floor(255*r), (int)floor(255*g), (int)floor(255*b)
	);
	res->setHeader("Cache-Control", "no-cache");
  	res->setHeader("Content-Type", "text/html");
	res->println(render_buffer);
	delete [] render_buffer;
}


void simpleMode(HTTPRequest* req, HTTPResponse* res) {
	if (req->getMethod() == "POST") {
		httpsserver::ResourceParameters* params = req->getParams();
		auto fun = [=](std::string name) {
			std::string param = "0000";
			params->getQueryParameter(name, param);
			return constrain<float>((float)atoi(param.c_str())/15.f, 0, 1);
		};    
		ColorChannels channels = {fun("ch0"), fun("ch1"), fun("ch2"), fun("ch3")};
		knobs::turnOff();
		inputs::setAuto(webColorSpace, channels);
		outputs::writeOutput();
	}
	String templateStr = configuration::getResourceStr(resource_simple_template_html);
	const char* colorspaces[] = {"hsv", "hsl", "rgb"};
	const char* channels[][4] = {{"hue", "saturation", "value", "white"}, {"hue", "saturation", "lightness", "white"}, {"red", "green", "blue", "white"}};
	const char** channelsInCurrentColorspace = channels[0];
	String destColorspace = webColorSpace;
	for (int i=0;i<3;i++)
		if (destColorspace == colorspaces[i])
			channelsInCurrentColorspace = channels[i];
	ColorChannels filteredChannels = inputs::getAuto(webColorSpace);
	char* render_buffer = new char[simple_template_html_decompressed_size+256];
	sprintf(
		render_buffer, templateStr.c_str(), 
		channelsInCurrentColorspace[0],
		floatFilter15(filteredChannels[0]),
		channelsInCurrentColorspace[1],
		floatFilter15(filteredChannels[1]),
		channelsInCurrentColorspace[2],
		floatFilter15(filteredChannels[2]),
		whiteKnobEnabled ? "" : "style=\"display: none;\"",
		channelsInCurrentColorspace[3],
		floatFilter15(filteredChannels[3])
	);
	res->setHeader("Cache-Control", "no-cache");
  	res->setHeader("Content-Type", "text/html");
	res->println(render_buffer);
	delete [] render_buffer;
}


void handleIndex(HTTPRequest* req, HTTPResponse* res) {
  res->setHeader("Content-Type", "text/html");
  res->println("<meta http-equiv=\"refresh\" content=\"0; url=/index.html\">");
}


void sendFavicon(HTTPRequest* req, HTTPResponse* res) {
	server::sendDecompressedData(res, resource_favicon_svg);
}


void configureServer() {
    server::configure();	
	server::addCallback("/", "GET", handleIndex);
	server::addCallback("/config.json", "GET", sendConfiguration);
	server::addCallback("/favicon.ico", "GET", sendFavicon);
	server::addCallback("/config.json", "POST", recvConfiguration);
	server::addCallback("/assert_json", "POST", customValidator);
	server::addCallback("/networks.json", "GET", sendNetworks);
	server::addCallback("/connect_to", "POST", connectToNetworkEndpoint);
	server::addCallback("/refresh_networks", "GET", autoScanWifiEndpoint);
	server::addCallback("/open_access_point", "GET", openAccessPointEndpoint);
	server::addCallback("/color.json", "GET", sendColors);
	server::addCallback("/color.json", "POST", setColors);
	server::addCallback("/simple.html", "GET", simpleMode);
	server::addCallback("/simple.html", "POST", simpleMode);
	server::addCallback("/update", "POST", update);
	server::addCallback("/version_info.json", "GET", getVersionInfo);
	server::addCallback("/favorite_color.html", "GET", renderFavoriteColor);
	server::addCallback("/get_favorites", "GET", sendFavorites);
	server::addCallback("/new_favorite", "GET", dumpFavorite);
	server::addCallback("/save_favorites", "POST", saveFavorites);
	server::addCallback("/apply_favorite", "GET", applyFavorite);
	server::addCallback("/invalidate_cache", "GET", invalidateCache);
	server::start();
}


unsigned long long int reset_timer = 0;
bool fanStatus = false;
uint8_t measureTemperature = 0;

void checkReset() {
	if (digitalRead(hardware_configuration.resetPin) == 0) {
		if (reset_timer == 0)
			reset_timer = millis();
		else if (millis() - reset_timer >= RESET_CONF_TIME) {
			configuration::resetConfiguration();
			ESP.restart();
		}
	} else 
		reset_timer = 0;
}


float readTemperature(float x) {
	float RT = x * THERMISTOR_IN_SERIES_RESISTOR / (1.f - x);
	float Tk = 1.0/(log(RT/THERMISTOR_R0)/4050.0 + 1.0/THERMISTOR_T0);   
	return Tk - 273.15;
}


temperature_sensor_handle_t temp_handle = NULL;

void initTemperature() {
	temperature_sensor_config_t temp_sensor = {
			.range_min = 20,
			.range_max = 100,
			.clk_src = TEMPERATURE_SENSOR_CLK_SRC_DEFAULT
	};
	temperature_sensor_install(&temp_sensor, &temp_handle);
}


void checkTemperature() {
	float tsens_out;
	temperature_sensor_enable(temp_handle);
	temperature_sensor_get_celsius(temp_handle, &tsens_out);
	temperature_sensor_disable(temp_handle);
	float temp_max = tsens_out;

	for (unsigned i=0;i<4;i++) {
		const auto& actions = hardware_configuration.thermistors[i];
		if (!actions.enabled) continue;
		float T = readTemperature(actions.read()/2);
		temp_max = max(temp_max, T);
	}

	fanStatus = ((fanStatus) && (temp_max > FAN_TURN_OFF_TEMP)) || ((!fanStatus) && (temp_max > FAN_TURN_ON_TEMP));
	digitalWrite(hardware_configuration.fanPin, fanStatus ? HIGH : LOW);
}


void setup() {
	Serial.begin(115200);  
	Serial.println("Initialization");
	randomSeed(29615);
	hardware::detectHardware();
	SPIFFS.begin(true);
	initTemperature();
	initLedC();
	updateModules();
	wifi::fastInit();
	knobs::check(true);
	knobs::attachTimer();
	configureServer();
}

unsigned long long int rareChecksTime = 0;

void loop() {

	if (wifi::connected()) {
		unsigned long long int ts = 0;
		unsigned i = 0;
		do {
			unsigned long long int t = millis();
			server::loop();
			ts = millis() - t;
		} while (ts > 1 && i++ < 100);
	}

	for (auto fun: taskQueue) 
		fun();
	taskQueue.clear();

	checkReset();
	
	if (rareChecksTime == 0 || millis() - rareChecksTime > RARE_CHECKS_INTERVAL) {
		wifi::checkConnection();
		checkTemperature();
		rareChecksTime = millis();
	}

}
