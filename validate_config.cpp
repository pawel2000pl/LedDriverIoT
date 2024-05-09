#include <cstdio>
#include <cstdlib>
#include <iostream>

#include "main/validate_json.h"

// g++ validate_config.cpp main/validate_json.cpp && ./a.out

#define JSON_CONFIG_BUF_SIZE (16*1024)

StaticJsonDocument<JSON_CONFIG_BUF_SIZE> configuration;
StaticJsonDocument<JSON_CONFIG_BUF_SIZE> schema;

void printError(const DeserializationError err) {
    if (err == DeserializationError::Ok) {puts("Reading ok"); return;}
    if (err == DeserializationError::EmptyInput) puts("Empty input");
    else if (err == DeserializationError::IncompleteInput) puts("Incomplete input");
    else if (err == DeserializationError::InvalidInput) puts("Invalid input");
    else if (err == DeserializationError::NoMemory) puts("No memory (configuration too long)");
    else if (err == DeserializationError::TooDeep) puts("Configuration too deep (so it must be invalid)");
    else puts("Cannot save configuration because no.");
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
    configuration.garbageCollect();

    fptr = fopen("resources/config.schema.json", "r"); 
    size = fread(buf, 1, JSON_CONFIG_BUF_SIZE-1, fptr);
    fclose(fptr);
    buf[size] = 0;
    printError(deserializeJson(schema, String(buf)));
    schema.garbageCollect();

    puts(validateJson(configuration, schema).c_str());

    return 0;
}