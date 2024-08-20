#pragma once

#include <string>
#include <filesystem>

#include "tftp.hpp"
#include "inmessage.hpp"


struct Client 
{
    sockaddr_in sockaddr;
    std::shared_ptr<IncommingMessage> msgin;
    SOCKET sock;
    unsigned long blksize;
    unsigned long tsize;
    std::filesystem::path file;
    unsigned long timeout;
};
