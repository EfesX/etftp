#include <memory>

#include <argparse/argparse.hpp>

#include "tftp_server.hpp"
#include "settings.hpp"

#include "spdlog/spdlog.h"
#include "spdlog/common.h"
#include "spdlog/sinks/stdout_color_sinks.h"
#include "spdlog/sinks/basic_file_sink.h"

#include "signal.h"

int main(int argc, char *argv[]) 
{
    auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
    auto file_sink = std::make_shared<spdlog::sinks::basic_file_sink_mt>("logs/log_" + std::to_string(std::time(0)) + ".txt", true);
    spdlog::logger logger("ETFTPServer", {console_sink, file_sink});
    std::shared_ptr<spdlog::logger> mlogger = std::make_shared<spdlog::logger>(logger);
    spdlog::set_default_logger(mlogger);

    spdlog::set_level(spdlog::level::debug);
    spdlog::info("Server initialization...");

    try {
        Settings::Instance().FromConfigFile(CONFIG_FILENAME);
    } catch(const std::exception& err) {
        spdlog::warn(err.what());
        spdlog::warn("Error happened during parse " + std::string(CONFIG_FILENAME));
        spdlog::warn("Command line arguments will be used for configuration");
    }

    try {
        Settings::Instance().FromArguments(argc, argv);
    } catch(const std::exception& err) {
        spdlog::warn(err.what());
        spdlog::warn("Error happened during parse command line arguments");
        spdlog::warn("Default values will be used for config");
    }

    spdlog::info("Initialization done");
    spdlog::info("Current settings:");
    spdlog::info("Port                       : {}", Settings::Instance().port);
    spdlog::info("Server root directory      : {}", Settings::Instance().rootdir);
    spdlog::info("Timeout of receive packets : {}", Settings::Instance().timeout);
    spdlog::info("Attempts of sending packets: {}", Settings::Instance().attempts);
    spdlog::info("Default block size         : {}", Settings::Instance().blksize);
    spdlog::info("Allow to create files      : {}", Settings::Instance().fcreate);
    spdlog::info("Allow to overwrite files   : {}", Settings::Instance().foverwrite);

    TFTPServer server(Settings::Instance().rootdir, Settings::Instance().port);

    signal(SIGINT, [](int sig){
        spdlog::info("Exiting");
        exit(EXIT_SUCCESS);
    });
    
    try
    {
        server.Run();
    }
    catch(const std::exception& err)
    {
        spdlog::error(err.what());
        spdlog::error("Server error. Exit.");
        exit(EXIT_FAILURE);
    }

    return EXIT_FAILURE;
}
