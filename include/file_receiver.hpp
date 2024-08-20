#pragma once

#include "tftp.hpp"
#include "listener.hpp"

#include <spdlog/spdlog.h>
#include "spdlog/fmt/bin_to_hex.h"

class FileReceiver : public Listener {
private:
    int ReceiveData(Client& client, char* data, uint16_t nblock)
    {
#ifndef __unix__
        fd_set fds;
        FD_ZERO(&fds);
        FD_SET(client.sock, &fds);

        timeval tv;
        tv.tv_sec  = Settings::Instance().timeout;
        tv.tv_usec = 0;

        int n = select(client.sock, &fds, NULL, NULL, &tv);

        if(n == 0)
        {
            SendError(client, ERR_NOT_DEFINED, "Data Waiting Timeout");
            return -1;
        }

        if(n == -1)
        {
            SendError(client, ERR_NOT_DEFINED, "Data Waiting Error");
            return -1;
        }
#endif

        socklen_t len = sizeof(client.sockaddr);
        int sz = recvfrom(client.sock, data, client.blksize + 4, 0, (struct sockaddr*) &client.sockaddr, &len);

        if(sz < 0)
        {
            spdlog::warn("Amount of received data from socket is invalid: {}", sz);
            SendError(client, ERR_NOT_DEFINED, "Something Wrong Happened");
            return -1;
        }

        if(FrameViewer::Opcode(data) != OPCODE_DATA_PACKET)
        {
            spdlog::warn("Wrong opcode was detected in received packet");
            SendError(client, ERR_NOT_DEFINED, "Wrong OPCODE");
            return -1;
        }

        if(FrameViewer::Block(data) != nblock)
        {
            spdlog::warn("Wrong block number. Expected: {}. Received: {} ({:04x})", nblock, FrameViewer::Block(data), FrameViewer::Block(data));
            spdlog::debug("{:02x} {:02x} {:02x} {:02x} {:02x} {:02x} {:02x} {:02x}", 
                data[0], data[1], data[2], data[3],
                data[4], data[5], data[6], data[7]
            );
            SendError(client, ERR_NOT_DEFINED, "Wrong Block Number");
            return -1;
        }

        return sz;

    }
public:
    bool Notify(Client& client) override 
    {
        if(FrameViewer::Opcode(client.msgin->data) != OPCODE_WRITE_REQUEST) return false;

        char* buffer = (char*) std::malloc(client.blksize + 4);

        uint16_t nblock = 1;

        uint16_t total_blocks = client.tsize / client.blksize;
        total_blocks += ((client.tsize % client.blksize) ? 1 : 0);

        std::ofstream ofs(client.file, std::ios::out | std::ios::binary);
        
        int attempts = 0;

        while(total_blocks > 0)
        {
            int sz = ReceiveData(client, buffer, nblock);

            if(sz == -1)
            {
                if(attempts >= Settings::Instance().attempts)
                {
                    spdlog::warn("Receiving data error. Attemt: {}. Abort", attempts);
                    delete buffer;
                    ofs.close();
                    return true;
                }
                else
                {
                    spdlog::warn("Receiving data error. Attemt: {}", attempts);
                    attempts++;
                }
            }

            if(!ofs.is_open()) 
            {
                spdlog::error("File wasn't opened");
                SendError(client, ERR_DISK_FULL_OR_ALLOC_EXCEEDED, "File Not Opened");
                return true;
            }

            ofs.write(FrameViewer::Data(buffer), sz - 4);
            
            SendAck(client, nblock);

            spdlog::info("Received {} blocks. Remains {} blocks", nblock, total_blocks - 1);

            nblock++;
            total_blocks--;
        }

        ofs.close();
        delete buffer;
        return false;
    }
};
