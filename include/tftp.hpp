#pragma once

#include <iostream>

#include <string>
#include <map>
#include <memory>
#include <functional>

#ifndef __unix__
    #include <WinSock2.h>
    #include <WS2tcpip.h>

    #define socklen_t int
#else
    #include <sys/socket.h>
    #include <netinet/in.h>

    #define SOCKET int
    #define INVALID_SOCKET -1
#endif

#ifndef __FUNCSIG__
    #define __FUNCSIG__ __PRETTY_FUNCTION__
#endif

#pragma comment(lib, "Ws2_32.lib")

#define TFTP_DEFAULT_ROOTDIR        "."

#define TFTP_DEFAULT_RX_TIMEOUT     2
#define TFTP_DEFAULT_PORT           69
#define TFTP_DEFAULT_SEND_ATTEMPTS  3
#define TFTP_DEFAULT_BLKSIZE        1024

#define TFTP_DEFAULT_ALLOW_FILE_CREATE      false
#define TFTP_DEFAULT_ALLOW_FILE_OVERWRITE   false

#define TFTP_SERVER_BUFFER_SIZE 512
#define CONFIG_FILENAME         "config.json"

enum OPCODE 
{
    OPCODE_READ_REQUEST = 1,
    OPCODE_WRITE_REQUEST,
    OPCODE_DATA_PACKET,
    OPCODE_ACK,
    OPCODE_ERROR,
    OPCODE_OPTION_ACK
};

enum ERRORCODE 
{
    ERR_NOT_DEFINED = 0,
    ERR_FILE_NOT_FOUND,
    ERR_ACCESS_VIOLATION,
    ERR_DISK_FULL_OR_ALLOC_EXCEEDED,
    ERR_ILLEGAL_OPERATION,
    ERR_UNKNOWN_TRANSFER_ID,
    ERR_FILE_ALREADY_EXISTS,
    ERR_NO_SUCH_USER,
    ERR_WRONG_OPTION
};
