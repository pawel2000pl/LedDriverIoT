#include <WebServer.h>
#include "fastlz.h"

int sendDecompressedData(WebServer& server, const char* content_type, const void* compressed_buffer, size_t compressed_size, size_t max_decompressed_size) {
    uint8_t* decompressed_buffer = new uint8_t[max_decompressed_size+1];
    size_t decompressed_size = fastlz_decompress(compressed_buffer, compressed_size, decompressed_buffer, max_decompressed_size);
    if (decompressed_size == 0) {
        Serial.println("Błąd dekompresji");
        delete [] decompressed_buffer;
        return 0;
    }
    decompressed_buffer[decompressed_size] = 0;
    server.send(200, content_type, (const char*)decompressed_buffer);
    delete [] decompressed_buffer;
    return 1;
}
