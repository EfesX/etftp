#pragma once

#include <string>
#include <cstdint>
#include <atomic>
#include <list>
#include <memory>

#include <spdlog/spdlog.h>

#include "listener.hpp"

class TFTPServerRunner {
private:
    std::string m_root_dir;
    uint16_t m_port;
    char m_inbuf[TFTP_SERVER_BUFFER_SIZE];
    std::atomic<bool> m_stopped;

    std::list<std::shared_ptr<Listener>> listeners;

public:
    TFTPServerRunner(const std::string& root_dir, uint16_t port) : 
        m_root_dir(root_dir), m_port(port), m_stopped(false)
    {}

#ifdef __unix__
    void Run()
    {
        struct sockaddr_in addr;
        int bytes_read;

        int sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
        if(sock < 0)
        {
            throw std::runtime_error("Socket wasn't created");
        }

        addr.sin_family = AF_INET;
        addr.sin_port   = htons(Settings::Instance().port);
        addr.sin_addr.s_addr = htonl(INADDR_ANY);
        if(bind(sock, (struct sockaddr*) &addr, sizeof(addr)) < 0)
        {
            throw std::runtime_error("Socket bind error");
        }

        m_stopped.store(false);

        spdlog::info("Server running...");

        struct sockaddr_in si_other;
        socklen_t slen = sizeof(si_other);

        while(!m_stopped)
        {
            spdlog::info("Listenning socket...");
            int rcved = recvfrom(sock, m_inbuf, TFTP_SERVER_BUFFER_SIZE, 0, (struct sockaddr*) &si_other, &slen);
            if(rcved <= 0) 
            {
                spdlog::debug(__FUNCSIG__);
                spdlog::error("From socket received {} bytes", rcved);
                break;
            }

            std::shared_ptr<IncommingMessage> msg = std::make_shared<IncommingMessage>();
            msg->data = (char*) std::malloc(rcved);
            std::memmove((void*) msg->data, (void*) m_inbuf, (size_t) rcved);
            msg->len = rcved;
            msg->addrin = si_other;

            Client client;
            client.msgin    = msg;
            client.sockaddr = si_other;
            client.blksize  = static_cast<unsigned long>(Settings::Instance().blksize);
            client.timeout  = static_cast<unsigned long>(Settings::Instance().timeout);

            SOCKET sock = socket(AF_INET, SOCK_DGRAM, 0);
            if(sock == INVALID_SOCKET)
            {
                spdlog::warn("Created socket for client is invalid. Request won't be handle");
                continue;
            }

            client.sock = sock;

            for(auto listener : listeners)
            {
                if(listener->Notify(client) == true)
                {
                    //closesocket(client.sock);
                    spdlog::warn("Perhaps something wrong happened");
                    break;
                }
            }
        }

        spdlog::info("Server stopped");
        close(sock);
    }
#endif

#ifndef __unix__
    void Run()
    {
        WSADATA wsData;
        int ok = WSAStartup(MAKEWORD(2,2), &wsData);
        if(ok) 
        {
            WSACleanup();
            throw std::runtime_error("Error occured " + std::to_string(WSAGetLastError()));
        }

        SOCKET sock = socket(AF_INET, SOCK_DGRAM, 0);
        if(sock == INVALID_SOCKET)
        {
            closesocket(sock);
            WSACleanup();
            throw std::runtime_error("Error occured " + std::to_string(WSAGetLastError()));
        }

        in_addr ip2num;
        ok = inet_pton(AF_INET, "0.0.0.0", &ip2num);
        if(ok <= 0) 
        {
            closesocket(sock);
            WSACleanup();
            throw std::runtime_error("Error occured " + std::to_string(WSAGetLastError()));
        }

        sockaddr_in srvInfo;
        ZeroMemory(&srvInfo, sizeof(srvInfo));
        srvInfo.sin_family = AF_INET;
        srvInfo.sin_addr = ip2num;
        srvInfo.sin_port = htons(m_port);

        ok = bind(sock, (sockaddr*) &srvInfo, sizeof(srvInfo));
        if(ok != 0)
        {
            closesocket(sock);
            WSACleanup();
            throw std::runtime_error("Error occured " + std::to_string(WSAGetLastError()));
        }

        struct sockaddr_in si_other;
        int slen = sizeof(si_other);

        m_stopped.store(false);

        char buf[100];

        spdlog::info("Server running...");

        while(!m_stopped)
        {
            spdlog::info("Listenning socket...");
            int rcved = recvfrom(sock, m_inbuf, TFTP_SERVER_BUFFER_SIZE, 0, (struct sockaddr*) &si_other, &slen);
            if(rcved <= 0) 
            {
                spdlog::debug(__FUNCSIG__);
                spdlog::error("From socket received {} bytes", rcved);
                break;
            }

            inet_ntop(AF_INET, &((sockaddr_in*) &si_other)->sin_addr , buf, 100);
            spdlog::info("Request from client {}", std::string(buf));

            std::shared_ptr<IncommingMessage> msg = std::make_shared<IncommingMessage>();
            msg->data = (char*) std::malloc(rcved);
            std::memmove((void*) msg->data, (void*) m_inbuf, (size_t) rcved);
            msg->len = rcved;
            msg->addrin = si_other;

            Client client;
            client.msgin    = msg;
            client.sockaddr = si_other;
            client.blksize  = static_cast<unsigned long>(Settings::Instance().blksize);
            client.timeout  = static_cast<unsigned long>(Settings::Instance().timeout);

            SOCKET sock = socket(AF_INET, SOCK_DGRAM, 0);
            if(sock == INVALID_SOCKET)
            {
                spdlog::warn("Created socket for client is invalid. Request won't be handle");
                continue;
            }

            client.sock = sock;

            for(auto listener : listeners)
            {
                if(listener->Notify(client) == true)
                {
                    //closesocket(client.sock);
                    spdlog::warn("Perhaps something wrong happened");
                    break;
                }
            }
        }

        spdlog::info("Server stopped");
        closesocket(sock);
        WSACleanup();
    }
#endif

    void Stop()
    {
        spdlog::info("Server stopping...");
        m_stopped.store(true);
    }

    void Subscribe(std::shared_ptr<Listener> listener)
    {
        listeners.push_back(listener);
    }
};