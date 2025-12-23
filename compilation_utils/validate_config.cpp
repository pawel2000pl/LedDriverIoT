#include <cstdio>
#include <cstdlib>
#include <iostream>

#include "../main/src/validate_json.h"

//run from main project directory:
//g++ compilation_utils/validate_config.cpp main/src/validate_json.cpp -o validate_configuration

#define JSON_CONFIG_BUF_SIZE (64*1024)

JsonDocument configuration;
JsonDocument configurationCopy;
JsonDocument schema;

int result = 0;

void printError(const DeserializationError err) {
    if (err == DeserializationError::Ok) {puts("Reading ok"); return;}
    if (err == DeserializationError::EmptyInput) puts("Empty input");
    else if (err == DeserializationError::IncompleteInput) puts("Incomplete input");
    else if (err == DeserializationError::InvalidInput) puts("Invalid input");
    else if (err == DeserializationError::NoMemory) puts("No memory (configuration too long)");
    else if (err == DeserializationError::TooDeep) puts("Configuration too deep (so it must be invalid)");
    else puts("Cannot save configuration because no.");
    result = 1;
}

int main() {

    char buf[JSON_CONFIG_BUF_SIZE];
    int size;
    FILE *fptr;

    fptr = fopen("resources/default_config.json", "r"); 
    size = fread(buf, 1, JSON_CONFIG_BUF_SIZE-1, fptr);
    fclose(fptr);
    buf[size] = 0;
    printError(deserializeJson(configuration, String(buf)));
    configurationCopy = configuration;
 

    fptr = fopen("resources/config.schema.json", "r"); 
    size = fread(buf, 1, JSON_CONFIG_BUF_SIZE-1, fptr);
    fclose(fptr);
    buf[size] = 0;
    printError(deserializeJson(schema, String(buf)));

    std::string validateResult = validateJson(configuration, schema, schema["main"]);
    if (validateResult.length())
        result = 1;
    puts(validateResult.c_str());

    configuration.remove("wifi");
    configuration["filters"]["inputFilters"]["hue"].remove(8);
    validateResult = validateJson(configuration, schema, schema["main"], ".", configurationCopy);
    if (validateResult.length())
        result = 1;
    puts(validateResult.c_str());
    if (configuration["wifi"].is<JsonObject>())
        puts("Key restored");
    if (configuration["filters"]["inputFilters"]["hue"].size() == 9)
        puts("Array restored");

    return result;
}
