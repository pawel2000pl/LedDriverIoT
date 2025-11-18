#ifdef ARDUINO
	#include <Arduino.h>
	#define inttostr(i) String(i)
#else
	#include <sys/types.h>
	#define inttostr(i) std::to_string(i)
#endif

#include <regex.h>
#include "validate_json.h"


bool validateRange(const JsonVariantConst& object, const JsonVariantConst& schema) {
	float value = object.as<float>();
	if (schema["min_value"].is<float>() && value < schema["min_value"]) return 0;
	if (schema["max_value"].is<float>() && value > schema["max_value"]) return 0;
	return 1;
}

const char* json_simple_types[] = {"object", "array", "boolean", "int", "integer", "float", "bool", "string", NULL};

bool isSimpleJsonType(String typeName) {
	const char** iterator = json_simple_types;
	while (*iterator) {
		if (typeName == *iterator)
			return true;
		iterator++;
	}
	return false;
}

JsonDocument JsonEmpty;
JsonDocument JsonNull;
bool loadJsonEmpty() {
	String emptyJson = "{}";
	deserializeJson(JsonEmpty, emptyJson);
	String emptyNull = "null";
	deserializeJson(JsonNull, emptyNull);
	return 1;
}
bool JsonEmptyLoaded = loadJsonEmpty();

String validateJson(const JsonVariant& object, const JsonVariantConst& schema, const JsonVariantConst& objectType, String path, const JsonVariantConst& defaults) {

	bool objectTypeIsInline = objectType.is<const char*>();

	if (objectTypeIsInline && !isSimpleJsonType(objectType.as<String>()))
		return validateJson(object, schema, schema[objectType.as<String>()], path, defaults);
	
	const JsonVariantConst& objectSchema = objectTypeIsInline ? JsonEmpty.as<JsonVariantConst>() : objectType;
	String type = objectTypeIsInline ? objectType : objectSchema["type"].is<const char*>() ? objectSchema["type"] : String("object");

	if (type == "object") {
		if (!object.is<JsonObject>()) return "Invalid type: " + path + " expected: object";
		const auto& fields = (objectSchema["type"] == "object" ? objectSchema["fields"] : objectSchema).as<JsonObjectConst>();
		for (const JsonPairConst& keyValue: fields) {
			const auto& key = keyValue.key();
			if (object[key].isNull()) {
				if (defaults.isNull())
					return "Missing key: " + path + "/" + key.c_str();
				object[key] = defaults[key];
			}
			String result = validateJson(object[key], schema, keyValue.value(), path+"/"+key.c_str(), defaults[key]);
			if (result.length() != 0) return result;
		}
	}

	if (type == "array") {
		if (!object.is<JsonArray>()) return "Invalid type: " + path + " expected: array";
		const auto& item = objectSchema["item"];
		int size = object.size();
		int defaultsSize = defaults.size();
		if (!defaults.isNull() && size < defaultsSize && (objectSchema["min_length"].is<int>() || objectSchema["length"].is<int>())) {
			for (int i=size;i<defaultsSize;i++)
				object.add(defaults[i]);
			size = defaultsSize;
		}
		if (objectSchema["max_length"].is<int>() && size > objectSchema["max_length"]) return "Array too long: " + path;
		if (objectSchema["min_length"].is<int>() && size < objectSchema["min_length"]) return "Array too short: " + path;
		if (objectSchema["length"].is<int>() && size != objectSchema["length"]) return "Invalid array size: " + path;
		for (int i=0;i<size;i++) {
			String result = validateJson(object[i], schema, item, path+"/"+inttostr(i), defaults[i<defaultsSize?i:(defaultsSize-1)]);
			if (result.length() != 0) return result;
		}
	}

	if (type == "float") {
		if (!object.is<float>()) return "Invalid type: " + path + " expected: float";
		if (!validateRange(object, objectSchema)) return "Invalid range: " + path;
	}
	if (type == "integer") {
		if (!object.is<int>()) return "Invalid type: " + path + " expected: integer";
		if (!validateRange(object, objectSchema)) return "Invalid range: " + path + "/" + path;
	}
	if (type == "bool" || type == "boolean") {
		if (!object.is<bool>()) return "Invalid type: " + path + " expected: boolean";
	}
	if (type == "string" || type == "text") {
		if (!object.is<const char*>()) return "Invalid type: " + path + " expected: string";
		const JsonString& casted = object.as<JsonString>();
		if (objectSchema["max_length"].is<int>() && casted.size() > objectSchema["max_length"]) return "Text too long: " + path;
		if (objectSchema["min_length"].is<int>() && casted.size() < objectSchema["min_length"]) return "Text too short: " + path;
		if (objectSchema["regexp"].is<const char*>() || objectSchema["regex"].is<const char*>()) {
			String pattern = objectSchema["regexp"].is<const char*>() ? objectSchema["regexp"].as<String>() : objectSchema["regex"].as<String>();
			regex_t restrict;
			regcomp(&restrict, pattern.c_str(), REG_EXTENDED | REG_NOSUB);      
			int result = regexec(&restrict, casted.c_str(), 0, NULL, 0);
			if (result == REG_NOMATCH)
					return "Text does not match the required pattern: " + path ;
			else if (result != 0)
					return "Unexpected error during validation of regular expression of entry: " + path;
			regfree(&restrict);
		}
	}

	return "";
}
