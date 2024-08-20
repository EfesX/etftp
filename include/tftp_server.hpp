#pragma once

#include <iostream>
#include <string>
#include <atomic>
#include <memory>
#include <filesystem>

#include <spdlog/spdlog.h>

#include "tftp_server_runner.hpp"

#include "frame_checker.hpp"
#include "options_handler.hpp"
#include "file_sender.hpp"
#include "file_receiver.hpp"

class TFTPServer {
private:
    TFTPServerRunner runner;

public:
    TFTPServer(const std::string& root_dir, uint16_t port) : 
        runner(root_dir, port)
    {
        runner.Subscribe(std::make_shared<FrameChecker>(root_dir));
        runner.Subscribe(std::make_shared<OptionsHandler>());
        runner.Subscribe(std::make_shared<FileSender>());
        runner.Subscribe(std::make_shared<FileReceiver>());
    }

    void Run()
    {
        runner.Run();
    }

    void Stop()
    {
        runner.Stop();
    }

};
