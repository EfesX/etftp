#pragma once

#include <filesystem>

#include <spdlog/spdlog.h>

#include "listener.hpp"
#include "frame_viewer.hpp"
#include "frame.hpp"
#include "settings.hpp"


namespace fs = std::filesystem;

class FrameChecker : public Listener {
private:
    std::string m_root;
public:
    FrameChecker(const std::string& rootdir) : m_root(rootdir) {}

    bool Notify(Client& client) override {

        const char* data = client.msgin->data;

        OPCODE opcode = FrameViewer::Opcode(data);

        if((opcode != OPCODE_READ_REQUEST) && (opcode != OPCODE_WRITE_REQUEST)){
            spdlog::error("Invalid opcode: {}", static_cast<int>(opcode));
            SendError(client, ERR_ILLEGAL_OPERATION, "Unsupported Operation");
            return true;
        }

        if(opcode == OPCODE_READ_REQUEST)
        {
            auto file = fs::path(m_root + "/" + std::string(FrameViewer::SourceFile(data))).make_preferred();

            spdlog::info("Read request");
            spdlog::info("File        : {}", file.generic_string());

            if(!fs::exists(file))
            {
                spdlog::warn("Requested file doesn't exist");
                SendError(client, ERR_FILE_NOT_FOUND, "File Not Found Or No Access");
                return true;
            }
            else
            {
                client.tsize = fs::file_size(file);
                client.file = file;

                spdlog::info("File's size : {}", client.tsize);
            }
        }
        else if(opcode == OPCODE_WRITE_REQUEST)
        {
            auto file = fs::path(m_root + "/" + std::string(FrameViewer::DestinationFile(data))).make_preferred();

            spdlog::info("Write request");
            spdlog::info("File        : {}", file.generic_string());

            if(fs::exists(file))
            {
                if(!Settings::Instance().foverwrite)
                {
                    spdlog::warn("File exists. Overwriting isn't allowed");
                    SendError(client, ERR_ACCESS_VIOLATION, "File Already Exists. Overwriting isn't allowed");
                    return true;
                }
            }
            else
            {
                if(!Settings::Instance().fcreate)
                {
                    spdlog::warn("File doesn't exist. Creating files isn't allowed");
                    SendError(client, ERR_ACCESS_VIOLATION, "Creating files isn't allowed");
                    return true;
                }
            }
            
            client.tsize = 0;
            client.file  = file;
        }
        return false;
    }
};
