#pragma once

#include <HTTPRequest.hpp>
#include <HTTPResponse.hpp>

using HTTPResponse = httpsserver::HTTPResponse;
using HTTPRequest = httpsserver::HTTPRequest;

namespace server {

    class RequestReader {
    public:
        RequestReader(HTTPRequest* req);

        int read();
        size_t readBytes(char* buffer, size_t length);

    private:
        static const int bufferSize = 1024;

        HTTPRequest* request;
        int bufPos;
        int bufSize;
        bool bufEnded;
        unsigned char buffer[bufferSize];

        void readBuffer();
    };


    class ResponseWriter {
        public:
            ResponseWriter(HTTPResponse* res);
            ~ResponseWriter();

            size_t write(uint8_t c);
            size_t write(const uint8_t *buffer, size_t length);
        private:
            static const int bufferSize = 1024;

            HTTPResponse* response;
            int bufPos;
            unsigned char buffer[bufferSize];

            void writeBuffer();
    };

}
