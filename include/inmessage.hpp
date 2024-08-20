#pragma once

#include "tftp.hpp"

struct IncommingMessage {
    char* data;
    int len;
    sockaddr_in addrin;
};
