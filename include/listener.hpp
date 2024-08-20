#pragma once

#include "tftp.hpp"
#include "client.hpp"
#include "frame.hpp"
#include "frame_viewer.hpp"
#include "settings.hpp"

class Listener {
public:
    virtual bool Notify(Client& client) = 0;
    virtual ~Listener() {}

protected:
    virtual void SendError(Client& client, ERRORCODE errcode, const std::string& errmsg)
    {
        auto err_frame = TFTPFrameFactory::CreateError(errcode, errmsg);
        SendFrame(client, err_frame);
    }

    virtual void SendFrame(Client& client, std::shared_ptr<TFTPFrame> frame)
    {
        sendto(client.sock, frame->Data().data(), frame->Data().size(), 0, (struct sockaddr*) &client.sockaddr, sizeof(client.sockaddr));
    }

    virtual bool ReceiveAck(Client& client, int nblock)
    {
        char* buffer = (char*) std::malloc(client.blksize);

#ifndef __unix__
        fd_set fds;
        FD_ZERO(&fds);
        FD_SET(client.sock, &fds);

        timeval tv;
        tv.tv_sec  = static_cast<long>(Settings::Instance().timeout);
        tv.tv_usec = 0;

        int n = select(client.sock, &fds, NULL, NULL, &tv);

        if(n == 0)
        {
            SendError(client, ERR_NOT_DEFINED, "Ack Waiting Timeout");
            delete buffer;
            return false;
        }

        if(n == -1)
        {
            SendError(client, ERR_NOT_DEFINED, "Ack Waiting Error");
            delete buffer;
            return false;
        }
#endif

        socklen_t len = sizeof(client.sockaddr);
        int sz = recvfrom(client.sock, buffer, client.blksize, 0, (struct sockaddr*) &client.sockaddr, &len);

        if(sz < 0)
        {
            SendError(client, ERR_NOT_DEFINED, "Something Wrong Happened");
            return false;
        }

        if(FrameViewer::Opcode(buffer) != OPCODE_ACK)
        {
            SendError(client, ERR_NOT_DEFINED, "Wrong OPCODE");
            delete buffer;
            return false;
        }

        if(FrameViewer::Block(buffer) != nblock)
        {
            SendError(client, ERR_NOT_DEFINED, "Wrong Block Number");
            delete buffer;
            return false;
        }

        return true;
    }

    virtual void SendAck(Client& client, int nblock)
    {
        SendFrame(client, TFTPFrameFactory::CreateDataAck(nblock));
    }
};
