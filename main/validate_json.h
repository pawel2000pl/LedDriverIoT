#include <regex.h>
#include "json.h"

#ifdef ARDUINO
  #define inttostr(i) String(i)
#else
  #include <string>
  using String = std::string;
  #define inttostr(i) std::to_string(i)
#endif

bool validateRange(const JsonVariantConst& object, const JsonVariantConst& schema) {
  float value = object.as<float>();
  if (schema.containsKey("min_value") && value < schema["min_value"]) return 0;
  if (schema.containsKey("max_value") && value > schema["max_value"]) return 0;
  return 1;
}


String validateJson(const JsonVariantConst& object, const JsonVariantConst& schema, String path=".") {
  
  String type = schema.containsKey("type") ? schema["type"].as<JsonString>().c_str() : "object";

  if (type == "object") {
    if (!object.is<JsonObjectConst>()) return "Invalid type: " + path + " expected: object";
    const auto& fields = schema["fields"].as<JsonObjectConst>();
    for (const JsonPairConst& keyValue: fields) {
      const auto& key = keyValue.key();
      if (!object.containsKey(key)) return "Missing key: " + path + "/" + key.c_str();
      String result = validateJson(object[key], keyValue.value(), path+"/"+key.c_str());
      if (result.length() != 0) return result;
    }
  }

  if (type == "array") {
    if (!object.is<JsonArrayConst>()) return "Invalid type: " + path + " expected: array";
    const auto& item = schema["item"];
    const int size = object.size();
    if (schema.containsKey("max_length") && size > schema["max_length"]) return "Array too long: " + path;
    if (schema.containsKey("min_length") && size < schema["min_length"]) return "Array too short: " + path;
    if (schema.containsKey("length") && size != schema["length"]) return "Invalid array size: " + path;
    for (int i=0;i<size;i++) {
      String result = validateJson(object[i], item, path+"/"+inttostr(i));
      if (result.length() != 0) return result;
    }
  }

  if (type == "float") {
    if (!object.is<float>()) return "Invalid type: " + path + " expected: float";
    if (!validateRange(object, schema)) return "Invalid range: " + path;
  }
  if (type == "integer") {
    if (!object.is<int>()) return "Invalid type: " + path + " expected: integer";
    if (!validateRange(object, schema)) return "Invalid range: " + path + "/" + path;
  }
  if (type == "bool" || type == "boolean") {
    if (!object.is<bool>()) return "Invalid type: " + path + " expected: boolean";
  }
  if (type == "string") {
    if (!object.is<const char*>()) return "Invalid type: " + path + " expected: string";
    if (schema.containsKey("max_length") && object.size() > schema["max_length"]) return "String too long: " + path;
    if (schema.containsKey("regexp") || schema.containsKey("regex")) {
      String testString = object.as<String>();
      String pattern = schema.containsKey("regexp") ? schema["regexp"].as<String>() : schema["regex"].as<String>();
      regex_t restrict;
      regcomp(&restrict, pattern.c_str(), REG_EXTENDED | REG_NOSUB);      
      int result = regexec(&restrict, testString.c_str(), 0, NULL, 0);
      if (result == REG_NOMATCH)
          return "Text does not match the regular expression: " + path ;
      else if (result != 0)
          return "Unexpected error during validation of regular expression of entry: " + path;
      regfree(&restrict);
    }
  }

  return "";
}

