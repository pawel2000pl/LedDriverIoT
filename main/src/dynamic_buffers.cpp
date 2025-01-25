#include "dynamic_buffers.h"


namespace server {

    RequestReader::RequestReader(HTTPRequest* req) 
        : request(req), bufPos(0), bufEnded(false) {
        readBuffer();
    }


    int RequestReader::read() {
        if (bufPos < bufSize) return buffer[bufPos++];
        if (bufEnded) return -1;
        readBuffer();
        return buffer[bufPos++];
    }
    

    size_t RequestReader::readBytes(char* buffer, size_t length) {
        char* bufEnd = buffer + length;
        char* i = buffer - 1;
        while (++i < bufEnd && (!bufEnded))
            *i = (char)read();
        return i - buffer;
    }
    

    void RequestReader::readBuffer() {
        bufPos = 0;
        bufSize = 0;
        while ((bufSize < bufferSize) && (!request->requestComplete()))
            bufSize += request->readBytes(buffer + bufSize, bufferSize - bufSize);
        bufEnded = request->requestComplete();
    }


    ResponseWriter::ResponseWriter(HTTPResponse* res) 
        : response(res), bufPos(0) {
    }


    ResponseWriter::~ResponseWriter() {
        writeBuffer();
    }


    size_t ResponseWriter::write(uint8_t c) {
        buffer[bufPos++] = c;
        if (bufPos >= bufferSize)
            writeBuffer();
        return 1;
    }


    size_t ResponseWriter::write(const uint8_t *buffer, size_t length) {
        const uint8_t* bufEnd = buffer + length;
        const uint8_t* i = buffer - 1;
        while (++i < bufEnd)
            write(*i);
        return i - buffer;
    }


    void ResponseWriter::writeBuffer() {
        if (bufPos > 0)
            response->write(buffer, bufPos);
        bufPos = 0;
    }

}
