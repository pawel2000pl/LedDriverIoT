#include <Arduino.h>
#include <WebServer.h>
#include <FS.h>
#include <SPIFFS.h>
#include <Update.h>
#include <functional>
#include <list>
#include <driver/temperature_sensor.h>
#include <ArduinoJson.h>

#include "constrain.h"
#include "resources.h"
#include "conversions.h"
#include "validate_json.h"
#include "fastlz.h"
#include "ledc_driver.h"
#include "hardware_configuration.h"
#include "light_pipeline.h"
#include "knobs.h"
#include "wifi.h"

#define FAN_TURN_ON_TEMP 70
#define FAN_TURN_OFF_TEMP 50
#define THERMISTOR_CONST 4050.0f
#define THERMISTOR_R0 47000
#define THERMISTOR_IN_SERIES_RESISTOR 47000
#define THERMISTOR_T0 (25.0f + 273.15f)
#define RESET_CONFIGURATION_PIN (D3)

#define CONFIGURATION_FILENAME "/configuration.json"
#define FAVORITES_FILENAME "/favorites.json"
#define JSON_CONFIG_BUF_SIZE (16*1024)
#define JSON_FAVORITES_BUF_SIZE (64*64)

using FavoriteJsonDoc = StaticJsonDocument<JSON_FAVORITES_BUF_SIZE>;

StaticJsonDocument<JSON_CONFIG_BUF_SIZE> configuration;
StaticJsonDocument<JSON_CONFIG_BUF_SIZE> defaultConfiguration;
StaticJsonDocument<JSON_CONFIG_BUF_SIZE> configSchema;
StaticJsonDocument<JSON_FAVORITES_BUF_SIZE> defaultFavorites;
StaticJsonDocument<1024> versionInfo;

std::list<std::function<void()>> taskQueue;

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


int sendDecompressedData(WebServer& server, const char* content_type, const void* compressed_buffer, size_t compressed_size, size_t max_decompressed_size) {
		uint8_t* decompressed_buffer = new uint8_t[max_decompressed_size+1];
		size_t decompressed_size = fastlz_decompress(compressed_buffer, compressed_size, decompressed_buffer, max_decompressed_size);
		if (decompressed_size == 0) {
				sendError("Decompression error");
				delete [] decompressed_buffer;
				return 0;
		}
		decompressed_buffer[decompressed_size] = 0;
		server.sendHeader("Cache-Control", "max-age=604800");
		server.send(200, content_type, (const char*)decompressed_buffer);
		delete [] decompressed_buffer;
		return 1;
}


String assertConfiguration() {
	return validateJson(configuration, configSchema, configSchema["main"], ".", defaultConfiguration); 
}


void loadDefautltConfiguration() {
	uint8_t decompressed_buffer[default_config_json_decompressed_size+1];
	size_t decompressed_size = fastlz_decompress(default_config_json_data, default_config_json_size, decompressed_buffer, default_config_json_decompressed_size);
	decompressed_buffer[decompressed_size] = 0;
	deserializeJson(defaultConfiguration, String((const char*)decompressed_buffer));
}


void loadDefautltFavorites() {
	uint8_t decompressed_buffer[default_favorites_json_decompressed_size+1];
	size_t decompressed_size = fastlz_decompress(default_favorites_json_data, default_favorites_json_size, decompressed_buffer, default_favorites_json_decompressed_size);
	decompressed_buffer[decompressed_size] = 0;
	deserializeJson(defaultFavorites, String((const char*)decompressed_buffer));
}


void loadVersionInfo() {
	uint8_t decompressed_buffer[version_json_decompressed_size+1];
	size_t decompressed_size = fastlz_decompress(version_json_data, version_json_size, decompressed_buffer, version_json_decompressed_size);
	decompressed_buffer[decompressed_size] = 0;
	deserializeJson(versionInfo, String((const char*)decompressed_buffer));
	versionInfo["date"] = __DATE__;
	versionInfo["time"] = __TIME__;
}


void loadConfiguration() {
	File configFile = SPIFFS.open(CONFIGURATION_FILENAME, "r");
	if (configFile) {
		String buf = configFile.readString();
		configFile.close();
		DeserializationError err = deserializeJson(configuration, buf);
		if (err != DeserializationError::Ok || assertConfiguration().length())
			configuration = defaultConfiguration;
		knobs::updateConfiguration(configuration);
		pipeline::updateConfiguration(configuration);
		wifi::updateConfiguration(configuration);
	}
}


void loadConfigSchema() {
	uint8_t decompressed_buffer[config_schema_json_decompressed_size+1];
	size_t decompressed_size = fastlz_decompress(config_schema_json_data, config_schema_json_size, decompressed_buffer, config_schema_json_decompressed_size);
	decompressed_buffer[decompressed_size] = 0;
	deserializeJson(configSchema, String((const char*)decompressed_buffer));
}


void resetConfiguration() {
	SPIFFS.remove(CONFIGURATION_FILENAME);
	loadConfiguration();
}


void saveConfiguration() {
	char* buf = new char[JSON_CONFIG_BUF_SIZE+1];
	unsigned size = serializeJson(configuration, buf, JSON_CONFIG_BUF_SIZE);
	buf[size] = 0;
	File configFile = SPIFFS.open(CONFIGURATION_FILENAME, "w");
	configFile.println(buf);
	configFile.close();
	delete [] buf;
}


void sendConfiguration() {
	char* buf = new char[JSON_CONFIG_BUF_SIZE+1];
	unsigned size = serializeJson(configuration, buf, JSON_CONFIG_BUF_SIZE);
	buf[size] = 0;
	server.sendHeader("Cache-Control", "no-cache");
	server.send(200, default_config_json_mime_type, buf);
	delete [] buf;
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
		loadConfiguration();
		sendDeserializationError(err);
		delete testJson;
		return;
	}
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
	const String assertMessage = validateJson(data, configSchema, configSchema["wifi-entry"]);
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
	DeserializationError err = deserializeJson(configuration, rawData);
	if (err != DeserializationError::Ok) {
		loadConfiguration();
		sendDeserializationError(err);
		return;
	}

	const String assertMessage = assertConfiguration();
	if (assertMessage != "") {
		sendError(assertMessage, 422);
		return;
	}
	if (save)
		saveConfiguration();
	else
		loadConfiguration();
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
	char buf[256];
	unsigned int size = serializeJson(versionInfo, buf, 255);
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
	DynamicJsonDocument favoritesDoc(JSON_FAVORITES_BUF_SIZE);
	JsonArray favorites;

	File favoritesFile = SPIFFS.open(FAVORITES_FILENAME, "r");
	if (favoritesFile) {
		String buf = favoritesFile.readString();
		favoritesFile.close();
		DeserializationError err = deserializeJson(favoritesDoc, buf);
		if (err != DeserializationError::Ok || validateJson(favoritesDoc, configSchema, configSchema["favorites-list"]).length())
			favorites = defaultFavorites.as<JsonArray>();
		else
			favorites = favoritesDoc.as<JsonArray>();
	} else favorites = defaultFavorites.as<JsonArray>();

	String colorspace = configuration["channels"]["webMode"];
	DynamicJsonDocument response(JSON_FAVORITES_BUF_SIZE);
	JsonArray reponseArray = response.to<JsonArray>();
	unsigned size = favorites.size();
	for (unsigned i=0;i<size;i++) {
		JsonObject item = reponseArray.createNestedObject();
		String code = favorites[i].as<String>();
		item["code"] = code;
		ColorChannels channels = pipeline::favoriteColorPreview(colorspace, code);
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
	String dumped = pipeline::dumpFavoriteColor(useWhite);
	ColorChannels channels = pipeline::getAuto(configuration["channels"]["webMode"]);
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
	String assertMessage = validateJson(data, configSchema, configSchema["favorites-list"]);
	if (assertMessage != "") {
		sendError(assertMessage, 422);
		return;
	}
	char* buf = new char[JSON_FAVORITES_BUF_SIZE+1];
	unsigned size = serializeJson(data, buf, JSON_FAVORITES_BUF_SIZE);
	buf[size] = 0;
	File favoritesFile = SPIFFS.open(FAVORITES_FILENAME, "w");
	favoritesFile.println(buf);
	favoritesFile.close();
	delete [] buf;
	server.sendHeader("Cache-Control", "no-cache");
	sendOk();
}


void applyFavorite() {
	String code = server.hasArg("code") ? server.arg("code") : "000000000";
	knobs::turnOff();
	pipeline::applyFavoriteColor(code);
	pipeline::writeOutput();
	server.sendHeader("Cache-Control", "no-cache");
	sendOk();
}


void sendColors() {
	ColorChannels channels = pipeline::getAuto(configuration["channels"]["webMode"]);
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
	String assertMessage = validateJson(data, configSchema, configSchema["color-channels"]);
	if (assertMessage != "") {
		sendError(assertMessage, 422);
		return;
	}
	pipeline::setAuto(configuration["channels"]["webMode"], {data[0].as<float>(), data[1].as<float>(), data[2].as<float>(), data[3].as<float>()});
	pipeline::writeOutput();
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
	pipeline::applyFavoriteColor(code);
	pipeline::writeOutput();
	knobs::turnOff();

	const auto& channelsMode = configuration["channels"]["webMode"];
	ColorChannels filteredChannels = pipeline::getAuto(channelsMode);

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
		pipeline::setAuto(configuration["channels"]["webMode"], channels);
		pipeline::writeOutput();
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
	String destColorspace = configuration["channels"]["webMode"];
	for (int i=0;i<3;i++)
		if (destColorspace == colorspaces[i])
			channelsInCurrentColorspace = channels[i];
	ColorChannels filteredChannels = pipeline::getAuto(configuration["channels"]["webMode"]);
	sprintf(
		render_buffer, (const char*)decompressed_buffer, 
		channelsInCurrentColorspace[0],
		floatFilter15(filteredChannels[0]),
		channelsInCurrentColorspace[1],
		floatFilter15(filteredChannels[1]),
		channelsInCurrentColorspace[2],
		floatFilter15(filteredChannels[2]),
		configuration["hardware"]["enbleWhiteKnob"].as<bool>() ? "" : "style=\"display: none;\"",
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
	server.on("/", HTTP_GET, [&server](){server.sendHeader("Cache-Control", "max-age=604800"); server.send(200, "text/html", "<meta http-equiv=\"refresh\" content=\"0; url=/index.html\">");});
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
	if (digitalRead(RESET_CONFIGURATION_PIN) == 0) {
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
		const auto& actions = THERMISTOR_HARDWARE_ACTIONS[i];
		if (!actions.enabled) continue;
		float T = readTemperature(actions.read()/2);
		temp_max = max(temp_max, T);
	}

	fanStatus = ((fanStatus) && (temp_max > FAN_TURN_OFF_TEMP)) || ((!fanStatus) && (temp_max > FAN_TURN_ON_TEMP));
	digitalWrite(FAN_PIN, fanStatus ? HIGH : LOW);
}


void setup() {
	Serial.begin(115200);  
	detectHardware();
	pinMode(RESET_CONFIGURATION_PIN, INPUT_PULLUP);
	initTemperature();
	initLedC();
	SPIFFS.begin(true);
	loadVersionInfo();
	loadConfigSchema();
	loadDefautltConfiguration();
	loadDefautltFavorites();
	loadConfiguration();
	wifi::fastInit(false);
	configureServer();
}

unsigned loopNumber = 1;

void loop() {

	knobs::check();

	unsigned long long int ts = 0;
	unsigned i = 0;
	do {
		unsigned long long int t = millis();
		server.handleClient();
		ts = millis() - t;
	} while (ts > 1 && i++ < 100);

	for (auto fun: taskQueue) 
		fun();
	taskQueue.clear();

	checkReset();
	if ((loopNumber++ & 255) == 0) {
		wifi::checkConnection();
		checkTemperature();
	}

}
