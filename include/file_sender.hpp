#pragma once

#include <fstream>

#include <spdlog/spdlog.h>

#include "listener.hpp"
#include "frame_viewer.hpp"

class FileSender : public Listener {
private:
public:
    bool Notify(Client& client) override {
        if(FrameViewer::Opcode(client.msgin->data) != OPCODE_READ_REQUEST) return false;

        std::ifstream ifs(client.file, std::ios::in | std::ios::binary);
        
        if(!ifs.is_open()) 
        {
            spdlog::error("File wasn't opened");
            SendError(client, ERR_FILE_NOT_FOUND, "File Not Opened");
            return true;
        }

        char *buffer = static_cast<char*>(std::malloc(client.blksize));
        buffer[client.blksize - 1] = 0x00;

        uint32_t nblock = 0;

        int len = sizeof(client.sockaddr);

        unsigned long current_size = 0;

        int pos = 0;

        int nblocks = 0;

        while(ifs.tellg() < client.tsize)
        {
            ifs.read(buffer, client.blksize);
            nblock++;

            int attempt = 0;

            while(attempt < Settings::Instance().attempts)
            {
                SendFrame(client, TFTPFrameFactory::CreateDataFrame(nblock, buffer, ifs.gcount()));

                if(ReceiveAck(client, nblock))
                {
                    break;
                }

                attempt++;
                spdlog::warn("Receiving ACK failed. Attempt: {} / {}", attempt, Settings::Instance().attempts);
            }

            if(attempt >= Settings::Instance().attempts)
            {
                spdlog::warn("Receiving ACK failed. Abort");
                ifs.close();
                delete buffer;
                return true;
            }

            nblocks++;

            if(ifs.tellg() == -1) 
            {
                spdlog::info("Sended {} blocks ({}/{} bytes)", nblocks, client.tsize, client.tsize);
                break;
            }
            else
            {
                spdlog::info("Sended {} blocks ({}/{} bytes)", nblocks, static_cast<int>(ifs.tellg()), client.tsize);
            }
        }

        ifs.close();
        delete buffer;
        return false;
    }
};
