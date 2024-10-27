#include <Arduino.h>
#include <FS.h>
#include <list>
#include <SPIFFS.h>
#include <Update.h>
#include <functional>
#include <WebServer.h>
#include <ArduinoJson.h>
#include <driver/temperature_sensor.h>

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

#define FAN_TURN_ON_TEMP 70
#define FAN_TURN_OFF_TEMP 50
#define THERMISTOR_CONST 4050.0f
#define THERMISTOR_R0 47000
#define THERMISTOR_IN_SERIES_RESISTOR 47000
#define THERMISTOR_T0 (25.0f + 273.15f)

#define RARE_CHECKS_INTERVAL (15000)


std::list<std::function<void()>> taskQueue;
String webColorSpace;
bool whiteKnobEnabled;

WebServer server(80);


void sendError(String message, int code = 400) {
	StaticJsonDocument<1024> messageData;
	char buf[1024];
	messageData["status"] = "error";
	messageData["message"] = message;
	unsigned int size = serializeJson(messageData, buf, 1023);
	buf[size] = 0;
	server.send(code, default_config_json_mime_type, buf);
}


void sendOk() {
	server.sendHeader("Cache-Control", "no-cache");
	server.send(200, default_config_json_mime_type, "{\"status\": \"ok\"}");
}


void sendCacheControlHeader() {
    int minAge = 432000; 
    int maxAge = 604800; 
    int randomAge = random(minAge, maxAge + 1);
    String headerValue = "max-age=" + String(randomAge);
    server.sendHeader("Cache-Control", headerValue);
}


int sendDecompressedData(WebServer& server, const char* content_type, const void* compressed_buffer, size_t compressed_size, size_t max_decompressed_size) {
		uint8_t* decompressed_buffer = new uint8_t[max_decompressed_size+1];
		size_t decompressed_size = fastlz_decompress(compressed_buffer, compressed_size, decompressed_buffer, max_decompressed_size);
		if (decompressed_size == 0) {
				sendError("Decompression error");
				delete [] decompressed_buffer;
				return 0;
		}
		decompressed_buffer[decompressed_size] = 0;
		sendCacheControlHeader();
		server.send(200, content_type, (const char*)decompressed_buffer);
		delete [] decompressed_buffer;
		return 1;
}


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


void sendConfiguration() {
	String buf = configuration::getConfigurationStr();
	server.sendHeader("Cache-Control", "no-cache");
	server.send(200, default_config_json_mime_type, buf.c_str());
}


void sendDeserializationError(DeserializationError err) {
		if (err == DeserializationError::EmptyInput) sendError("Empty input", 400);
		else if (err == DeserializationError::IncompleteInput) sendError("Incomplete input", 422);
		else if (err == DeserializationError::InvalidInput) sendError("Invalid input", 422);
		else if (err == DeserializationError::NoMemory) sendError("Out of memory (configuration too long)", 413);
		else if (err == DeserializationError::TooDeep) sendError("Configuration too deep (so it must be invalid)", 413);
		else sendError("Cannot save configuration because no.", 400);
}


void customValidator() {
	String rawData = server.arg("plain");
	StaticJsonDocument<2*JSON_CONFIG_BUF_SIZE> *testJson = new StaticJsonDocument<2*JSON_CONFIG_BUF_SIZE>();
	DeserializationError err = deserializeJson(*testJson, rawData);
	if (err != DeserializationError::Ok) {
		sendDeserializationError(err);
		delete testJson;
		return;
	}
	const auto configSchema = configuration::getConfigSchema();
	const String assertMessage = testJson->containsKey("type") ? validateJson((*testJson)["data"], configSchema, configSchema[(*testJson)["type"].as<String>()]) : validateJson((*testJson)["data"], (*testJson)["schema"], (*testJson)["schema"]["main"]);
	delete testJson;
	if (assertMessage != "") {
		sendError(assertMessage, 422);
		return;
	}
	sendOk();
}


void connectToNetworkEndpoint() {
	String rawData = server.arg("plain");
	StaticJsonDocument<768> data;
	DeserializationError err = deserializeJson(data, rawData);
	if (err != DeserializationError::Ok) {
		sendDeserializationError(err);
		return;
	}
	const String assertMessage = configuration::assertJson(data, "wifi-entry");
	if (assertMessage != "") {
		sendError(assertMessage, 422);
		return;
	}
	sendOk();
	const String ssid = data["ssid"].as<String>();
	const String password = data["password"].as<String>();
	taskQueue.push_back([ssid, password](){wifi::connectToNetwork(ssid, password);});
}


void autoScanWifiEndpoint() {
	sendOk(); 
	taskQueue.push_back(wifi::scanNetworks);
}


void openAccessPointEndpoint() {
	sendOk(); 
	taskQueue.push_back(wifi::openAccessPoint);
}


void recvConfiguration(bool save=true) {
	String rawData = server.arg("plain");
	DynamicJsonDocument configuration(JSON_CONFIG_BUF_SIZE);
	DeserializationError err = deserializeJson(configuration, rawData);
	if (err != DeserializationError::Ok) {
		sendDeserializationError(err);
		return;
	}
	const String assertMessage = configuration::assertConfiguration(configuration);
	if (assertMessage != "") {
		sendError(assertMessage, 422);
		return;
	}
	if (save) {
		configuration::setConfiguration(configuration);
		updateModules(configuration);
	}
	sendOk();
}


void update() {
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
}


void invalidateCache() {
	server.sendHeader("Clear-Site-Data", "\"cache\"");
	server.sendHeader("Connection", "close");
	sendOk();
}


void updateEnd() {
	if (Update.hasError()) 
		sendError(Update.errorString());
	else {
		invalidateCache();
		taskQueue.push_back([](){ESP.restart();});
	}
}


void getVersionInfo() {
	char buf[JSON_VERSION_INFO_BUF_SIZE];
	unsigned int size = serializeJson(configuration::getVersionInfo(), buf, JSON_VERSION_INFO_BUF_SIZE-1);
	buf[size] = 0;
	server.sendHeader("Cache-Control", "no-cache");
	server.send(200, default_config_json_mime_type, buf);
}


void sendNetworks() {
	unsigned lengthSum = 0;
	const auto& scannedNetworks = wifi::getScannedNetworks();
	for (auto s : scannedNetworks) lengthSum += s.length();
	unsigned bufSize = lengthSum + scannedNetworks.size() + 32;
	DynamicJsonDocument networkList(bufSize);
	char* buf = new char[bufSize+1];
	for (auto s : scannedNetworks) networkList.add(s);
	unsigned size = serializeJson(networkList, buf, bufSize);
	buf[size] = 0;
	server.sendHeader("Cache-Control", "no-cache");
	server.send(200, default_config_json_mime_type, buf);
	delete [] buf;
}


void sendFavourites() {
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

	char* buf = new char[JSON_FAVORITES_BUF_SIZE];
	size = serializeJson(response, buf, JSON_FAVORITES_BUF_SIZE-1);
	buf[size] = 0;
	server.sendHeader("Cache-Control", "no-cache");
	server.send(200, default_config_json_mime_type, buf);
	delete [] buf;
}


void dumpFavorite(bool useWhite=false) {
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
	server.sendHeader("Cache-Control", "no-cache");
	server.send(200, default_config_json_mime_type, buf);
}


void saveFavorites() {
	DynamicJsonDocument data(JSON_FAVORITES_BUF_SIZE);
	String rawData = server.arg("plain");
	DeserializationError err = deserializeJson(data, rawData);
	if (err != DeserializationError::Ok) 
		return sendError("Deserialization error");
	String assertMessage = configuration::assertJson(data, "favorites-list");
	if (assertMessage != "") {
		sendError(assertMessage, 422);
		return;
	}
	configuration::setFavorites(data);
	server.sendHeader("Cache-Control", "no-cache");
	sendOk();
}


void applyFavorite() {
	String code = server.hasArg("code") ? server.arg("code") : "000000000";
	knobs::turnOff();
	inputs::applyFavoriteColor(code);
	outputs::writeOutput();
	server.sendHeader("Cache-Control", "no-cache");
	sendOk();
}


void sendColors() {
	ColorChannels channels = inputs::getAuto(webColorSpace);
	char buf[256];
	int size = sprintf(buf, "[%f, %f, %f, %f]", 
		channels[0],
		channels[1],
		channels[2],
		channels[3]
	);
	buf[size] = 0;
	server.sendHeader("Cache-Control", "no-cache");
	server.send(200, default_config_json_mime_type, buf);
}


void setColors() {
	StaticJsonDocument<256> data;
	String rawData = server.arg("plain");
	DeserializationError err = deserializeJson(data, rawData);
	if (err != DeserializationError::Ok) 
		return sendError("Deserialization error");
	String assertMessage = configuration::assertJson(data, "color-channels");
	if (assertMessage != "") {
		sendError(assertMessage, 422);
		return;
	}
	inputs::setAuto(webColorSpace, {data[0].as<float>(), data[1].as<float>(), data[2].as<float>(), data[3].as<float>()});
	outputs::writeOutput();
	knobs::turnOff();
	sendOk();   
}


int floatFilter15(float x) { 
	return (int)round(x * 15.f); 
};


void renderFavoriteColor() {
	uint8_t* decompressed_buffer = new uint8_t[favorite_color_template_html_decompressed_size+1];
	char* render_buffer = new char[favorite_color_template_html_decompressed_size+256];
	size_t decompressed_size = fastlz_decompress(favorite_color_template_html_data, favorite_color_template_html_size, decompressed_buffer, favorite_color_template_html_decompressed_size);
	if (decompressed_size == 0) {
			delete [] render_buffer;
			delete [] decompressed_buffer;
			sendError("Decompression error");
	}
	decompressed_buffer[decompressed_size] = 0;

	String code = server.hasArg("code") ? server.arg("code") : "000000000";
	inputs::applyFavoriteColor(code);
	outputs::writeOutput();
	knobs::turnOff();

	const auto& channelsMode = webColorSpace;
	ColorChannels filteredChannels = inputs::getAuto(channelsMode);

	float r, g, b;
	if (channelsMode == "rgb") rgbToRgb(filteredChannels[0], filteredChannels[1], filteredChannels[2], r, g, b);
	if (channelsMode == "hsl") hslToRgb(filteredChannels[0], filteredChannels[1], filteredChannels[2], r, g, b);
	if (channelsMode == "hsv") hsvToRgb(filteredChannels[0], filteredChannels[1], filteredChannels[2], r, g, b);

	sprintf(
		render_buffer, (const char*)decompressed_buffer,
		(int)floor(255*r), (int)floor(255*g), (int)floor(255*b)
	);
	server.sendHeader("Cache-Control", "no-cache");
	server.send(200, "text/html", (const char*)render_buffer);
	delete [] render_buffer;
	delete [] decompressed_buffer;
}


void simpleMode() {
	if (server.method() == HTTP_POST) {
		auto fun = [](String value) { return constrain<float>((float)atoi(value.c_str())/15.f, 0, 1);};    
		ColorChannels channels = {fun(server.arg("ch0")), fun(server.arg("ch1")), fun(server.arg("ch2")), fun(server.arg("ch3"))};
		knobs::turnOff();
		inputs::setAuto(webColorSpace, channels);
		outputs::writeOutput();
	}
	uint8_t* decompressed_buffer = new uint8_t[simple_template_html_decompressed_size+1];
	char* render_buffer = new char[simple_template_html_decompressed_size+256];
	size_t decompressed_size = fastlz_decompress(simple_template_html_data, simple_template_html_size, decompressed_buffer, simple_template_html_decompressed_size);
	if (decompressed_size == 0) {
			delete [] render_buffer;
			delete [] decompressed_buffer;
			sendError("Decompression error");
	}
	decompressed_buffer[decompressed_size] = 0;
	const char* colorspaces[] = {"hsv", "hsl", "rgb"};
	const char* channels[][4] = {{"hue", "saturation", "value", "white"}, {"hue", "saturation", "lightness", "white"}, {"red", "green", "blue", "white"}};
	const char** channelsInCurrentColorspace = channels[0];
	String destColorspace = webColorSpace;
	for (int i=0;i<3;i++)
		if (destColorspace == colorspaces[i])
			channelsInCurrentColorspace = channels[i];
	ColorChannels filteredChannels = inputs::getAuto(webColorSpace);
	sprintf(
		render_buffer, (const char*)decompressed_buffer, 
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
	server.sendHeader("Cache-Control", "no-cache");
	server.send(200, "text/html", (const char*)render_buffer);
	delete [] render_buffer;
	delete [] decompressed_buffer;
}


void configureServer() {
	server.enableDelay(false);
	server.on("/", HTTP_GET, [&server](){sendCacheControlHeader(); server.send(200, "text/html", "<meta http-equiv=\"refresh\" content=\"0; url=/index.html\">");});
	server.on("/config.json", HTTP_GET, sendConfiguration);
	server.on("/reset_configuration", HTTP_GET, [&server](){resetConfiguration(); sendOk();});
	server.on("/favicon.ico", HTTP_GET, [&server](){sendDecompressedData(server, resource_favicon_svg.mime_type, resource_favicon_svg.data, resource_favicon_svg.size, resource_favicon_svg.decompressed_size);});
	for (unsigned i=0;i<resources_count;i++)
		server.on(resources[i]->name, HTTP_GET, [i, &server](){sendDecompressedData(server, resources[i]->mime_type, resources[i]->data, resources[i]->size, resources[i]->decompressed_size);});  
	server.on("/config.json", HTTP_POST, [](){recvConfiguration(true);});
	server.on("/assert_config", HTTP_POST, [](){recvConfiguration(false);});
	server.on("/assert_json", HTTP_POST, customValidator);
	server.on("/networks.json", HTTP_GET, sendNetworks);
	server.on("/connect_to", HTTP_POST, connectToNetworkEndpoint);
	server.on("/refresh_networks", HTTP_GET, autoScanWifiEndpoint);
	server.on("/open_access_point", HTTP_GET, openAccessPointEndpoint);
	server.on("/color.json", HTTP_GET, sendColors);
	server.on("/color.json", HTTP_POST, setColors);
	server.on("/simple.html", HTTP_GET, simpleMode);
	server.on("/simple.html", HTTP_POST, simpleMode);
	server.on("/update", HTTP_POST, updateEnd, update);
	server.on("/version_info.json", HTTP_GET, getVersionInfo);
	server.on("/favorite_color.html", HTTP_GET, renderFavoriteColor);
	server.on("/get_favorites", HTTP_GET, sendFavourites);
	server.on("/new_favorite", HTTP_GET, [](){dumpFavorite(server.hasArg("white") && server.arg("white") != "0");});
	server.on("/save_favorites", HTTP_POST, saveFavorites);
	server.on("/apply_favorite", HTTP_GET, applyFavorite);
	server.on("/invalidate_cache", HTTP_GET, invalidateCache);
	server.begin();
}


unsigned long long int reset_timer = 0;
bool fanStatus = false;
uint8_t measureTemperature = 0;

void checkReset() {
	if (digitalRead(hardware_configuration.resetPin) == 0) {
		if (reset_timer == 0)
			reset_timer = millis();
		else if (millis() - reset_timer >= 10000) {
			SPIFFS.remove(CONFIGURATION_FILENAME);
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
			server.handleClient();
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
