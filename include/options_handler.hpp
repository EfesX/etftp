#pragma once

#include "listener.hpp"
#include "client.hpp"
#include "frame_viewer.hpp"
#include "settings.hpp"

#include <spdlog/spdlog.h>


class OptionsHandler : public Listener {
private:
    std::map<std::string, std::string> options;

public:
    bool Notify(Client& client)
    {
        std::map<std::string, std::string> opts;
        auto it = FrameViewer::OptionsBegin(client.msgin->data, client.msgin->len);

        for(it; !it->End(); )
        {
            auto option = it->Next();

            if(option.first == "blksize")
            {
                spdlog::info("Requested option >> {} : {}", option.first, option.second);
                client.blksize = strtol(option.second.c_str(), nullptr, 10);
                opts.emplace(option.first, option.second);
                spdlog::info("Option accepted");
            }
            else if(option.first == "tsize")
            {
                spdlog::info("Requested option >> {} : {}", option.first, option.second);
                opts.emplace(option.first, std::to_string(client.tsize));

                if(FrameViewer::Opcode(client.msgin->data) == OPCODE_WRITE_REQUEST)
                {
                    client.tsize = strtol(option.second.c_str(), nullptr, 10);
                }

                spdlog::info("Option accepted");
            }
            else if(option.first == "timeout")
            {
                spdlog::info("Requested option >> {} : {}", option.first, option.second);
                client.timeout = strtol(option.second.c_str(), nullptr, 10);
                opts.emplace(option.first, option.second);
                spdlog::info("Option accepted");
            }
            else
            {
                spdlog::warn("Requested wrong option >> {} : {}", option.first, option.second);
                SendError(client, ERR_WRONG_OPTION, "Wrong Option");
                spdlog::warn("Abort");
                return true;
            }
        }

        auto frame = TFTPFrameFactory::CreateOptionAck(opts);
        int attempt = 0;

        while(attempt < Settings::Instance().attempts)
        {
            SendFrame(client, frame);

            if(FrameViewer::Opcode(client.msgin->data) == OPCODE_READ_REQUEST)
            {
                if(ReceiveAck(client, 0))
                {
                    return false;
                }

                attempt++;
                spdlog::warn("Receiving ACK failed. Attempt: {} / {}", attempt, Settings::Instance().attempts);
            }
            else
            {
                return false;
            }
        }

        spdlog::warn("Receiving ACK failed. Abort");
        return true;
    }
};
