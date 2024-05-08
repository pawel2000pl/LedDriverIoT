#ifdef ARDUINO
  #include <Arduino.h>
  #define inttostr(i) String(i)
#else
  #include <sys/types.h>
  #define inttostr(i) std::to_string(i)
#endif

#include <regex.h>
#include "json.h"
#include "validate_json.h"

bool validateRange(const JsonVariantConst& object, const JsonVariantConst& schema) {
  float value = object.as<float>();
  if (schema.containsKey("min_value") && value < schema["min_value"]) return 0;
  if (schema.containsKey("max_value") && value > schema["max_value"]) return 0;
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

StaticJsonDocument<64> JsonEmpty;
bool loadJsonEmpty() {
  String emptyJson = "{}";
  deserializeJson(JsonEmpty, emptyJson);
  return 1;
}
bool JsonEmptyLoaded = loadJsonEmpty();

String validateJson(const JsonVariantConst& object, const JsonVariantConst& schema, const JsonVariantConst& objectType, String path) {

  bool objectTypeIsInline = objectType.is<String>();

  if (objectTypeIsInline && !isSimpleJsonType(objectType.as<String>()))
    return validateJson(object, schema, schema[objectType.as<String>()], path);
  
  const JsonVariantConst& objectSchema = objectTypeIsInline ? JsonEmpty.as<JsonVariantConst>() : objectType;
  String type = objectTypeIsInline ? objectType : objectSchema.containsKey("type") ? objectSchema["type"] : String("object");

  if (type == "object") {
    if (!object.is<JsonObjectConst>()) return "Invalid type: " + path + " expected: object";
    const auto& fields = objectSchema["fields"].as<JsonObjectConst>();
    for (const JsonPairConst& keyValue: fields) {
      const auto& key = keyValue.key();
      if (!object.containsKey(key)) return "Missing key: " + path + "/" + key.c_str();
      String result = validateJson(object[key], schema, keyValue.value(), path+"/"+key.c_str());
      if (result.length() != 0) return result;
    }
  }

  if (type == "array") {
    if (!object.is<JsonArrayConst>()) return "Invalid type: " + path + " expected: array";
    const auto& item = objectSchema["item"];
    const int size = object.size();
    if (objectSchema.containsKey("max_length") && size > objectSchema["max_length"]) return "Array too long: " + path;
    if (objectSchema.containsKey("min_length") && size < objectSchema["min_length"]) return "Array too short: " + path;
    if (objectSchema.containsKey("length") && size != objectSchema["length"]) return "Invalid array size: " + path;
    for (int i=0;i<size;i++) {
      String result = validateJson(object[i], schema, item, path+"/"+inttostr(i));
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
    if (objectSchema.containsKey("max_length") && casted.size() > objectSchema["max_length"]) return "Text too long: " + path;
    if (objectSchema.containsKey("min_length") && casted.size() < objectSchema["min_length"]) return "Text too short: " + path;
    if (objectSchema.containsKey("regexp") || objectSchema.containsKey("regex")) {
      String pattern = objectSchema.containsKey("regexp") ? objectSchema["regexp"].as<String>() : objectSchema["regex"].as<String>();
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

String validateJson(const JsonVariantConst& object, const JsonVariantConst& schema) {
  return validateJson(object, schema, schema["main"]);
}
