#pragma once

#include <argparse/argparse.hpp>

#include <fstream>
#include <nlohmann/json.hpp>

#include "tftp.hpp"

class Settings {
private:
    argparse::ArgumentParser program;

    Settings() : program("efestftp"){}
public:

    uint16_t port       = TFTP_DEFAULT_PORT;
    uint8_t  timeout    = TFTP_DEFAULT_RX_TIMEOUT;
    uint8_t  attempts   = TFTP_DEFAULT_SEND_ATTEMPTS;
    uint32_t blksize    = TFTP_DEFAULT_BLKSIZE;
    bool     fcreate    = TFTP_DEFAULT_ALLOW_FILE_CREATE;
    bool     foverwrite = TFTP_DEFAULT_ALLOW_FILE_OVERWRITE;
    std::string rootdir = TFTP_DEFAULT_ROOTDIR;

    static Settings& Instance()
    {
        static Settings instance;
        return instance;
    }

    void FromConfigFile(const std::string& filepath)
    {
        using json = nlohmann::json;

        std::ifstream conffile(filepath);
        if(!conffile.is_open()) throw std::runtime_error("Config File Not Opened");

        json config = json::parse(conffile);

        conffile.close();

        try{
            port       = config["port"].get<uint16_t>();
        }catch(...){}

        try{
            rootdir    = config["rootdir"].get<std::string>();
        }catch(...){}

        try{
            timeout    = config["timeout"].get<uint8_t>();
        }catch(...){}

        try{
            attempts   = config["attempts"].get<uint8_t>();
        }catch(...){}

        try{
            blksize    = config["blksize"].get<uint32_t>();
        }catch(...){}

        try{
            fcreate    = config["fcreate"].get<bool>();
        }catch(...){}

        try{
            foverwrite = config["foverwrite"].get<bool>();
        }catch(...){}
    }

    void FromArguments(int argc, char* argv[])
    {
        program.add_argument("-r", "--rootdir")
            .help("server's root directory")
            .action([&](const std::string& arg) { 
                rootdir = arg;
             })
            .default_value(TFTP_DEFAULT_ROOTDIR);

        program.add_argument("-p", "--port")
            .help("server's port")
            .action([&](const std::string& arg) { 
                port = std::stoul(arg, nullptr, 10);
            })
            .default_value(TFTP_DEFAULT_PORT);

        program.add_argument("-t", "--timeout")
            .help("timeout of waiting data and ack packets")
            .action([&](const std::string& arg) { 
                timeout = std::stoul(arg, nullptr, 10);
            })
            .default_value(TFTP_DEFAULT_RX_TIMEOUT);

        program.add_argument("-a", "--attempts")
            .help("amount attempts of transmittings data and ack packets")
            .action([&](const std::string& arg) { 
                attempts = std::stoul(arg, nullptr, 10);
            })
            .default_value(TFTP_DEFAULT_SEND_ATTEMPTS);

        program.add_argument("-bs", "--blksize")
            .help("default value of blksize option")
            .action([&](const std::string& arg) { 
                blksize = std::stoul(arg, nullptr, 10);
            })
            .default_value(TFTP_DEFAULT_BLKSIZE);

        program.add_argument("-fc", "--fcreate")
            .help("allow server to create files")
            .action([&](const std::string& arg) { 
                fcreate = true;
            })
            .default_value(TFTP_DEFAULT_ALLOW_FILE_CREATE)
            .flag();

        program.add_argument("-fo", "--foverwrite")
            .help("allow server to overwrite files")
            .action([&](const std::string& arg) { 
                foverwrite = true;
            })
            .default_value(TFTP_DEFAULT_ALLOW_FILE_OVERWRITE)
            .flag();

        program.add_description("The simplest TFTP server.\nAuthor: GuzhbaIS aka EfesX");
        program.add_epilog("...use it at your own risk...");

        program.parse_args(argc, argv);
    }
};
