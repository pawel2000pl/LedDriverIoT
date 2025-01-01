
#include "update.h"

#include <Update.h>
#include <Arduino.h>

#include "configuration.h"

#include "../server.h"
#include "../modules.h"

namespace endpoints {

    void update(HTTPRequest* req, HTTPResponse* res) {
        if (!Update.begin(UPDATE_SIZE_UNKNOWN)) {
            Update.printError(Serial);
            Update.abort();
            server::sendError(res, "Cannot begin the update", 500);
            return;
        }

        const unsigned max_buf_size = 16*1024;
        std::vector<unsigned char> buf;
        buf.resize(max_buf_size+4);
        unsigned char* bufPtr = buf.data();

        while (!req->requestComplete()) {
            unsigned size = 0;
            for (int i = 0; i < 32 && max_buf_size > size; i++)
                size += req->readBytes(bufPtr+size, max_buf_size-size);
            if (Update.write(bufPtr, size) != size) {
                Update.printError(Serial);
                Update.abort();
                server::sendError(res, "Update error (network error or update file is too large)", 400);
                return;
            }
        }

        if ((!Update.end(true)) || Update.hasError()) 
            server::sendError(res, Update.errorString(), 500);
        else {
            modules::taskQueue.push_back([](){ESP.restart();});
            server::sendOk(res);
        }
        Update.printError(Serial);   
    }


    void restart(HTTPRequest* req, HTTPResponse* res) {
        server::sendOk(res);
        modules::taskQueue.push_back([](){ESP.restart();});
    }

}
