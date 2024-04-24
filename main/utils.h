#include <WebServer.h>
#include "fastlz.h"

int sendDecompressedData(WebServer& server, const char* content_type, const void* compressed_buffer, size_t compressed_size, size_t max_decompressed_size) {
    // Utwórz bufor dla danych zdekompresowanych
    uint8_t decompressed_buffer[max_decompressed_size+1];

    // Dekompresuj dane
    size_t decompressed_size = fastlz_decompress(compressed_buffer, compressed_size, decompressed_buffer, max_decompressed_size);

    // Sprawdź, czy dekompresja się powiodła
    if (decompressed_size == 0) {
        Serial.println("Błąd dekompresji");
        return 0;
    }
    
    Serial.println("Dane zdekompresowane z powodzeniem");
    Serial.println((const char*)decompressed_buffer+2);
    decompressed_buffer[max_decompressed_size] = 0;
    // Wyślij dane zdekompresowane przez WebServer
    server.send(200, content_type, (const char*)decompressed_buffer);
    Serial.println("Dane wysłane z powodzeniem");
    return 1;
}
