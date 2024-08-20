#pragma once

#include <iostream>
#include <vector>
#include <map>

#include "tftp.hpp"

class TFTPFrame {
protected:
    std::vector<char> m_data;

public:
    TFTPFrame() = default;
    ~TFTPFrame() = default;

    template<typename...Args>
    void Add(Args...args ){
        throw std::runtime_error("Not Yet Implemented\n" + std::string(__FUNCSIG__));
        //static_assert(false);
    }

//    template<>
    void Add(char byte)
    {
        m_data.push_back(byte);
    }

//    template<>
    void Add(uint16_t value)
    {
        Add((char) (value >> 8));
        Add((char) value);
    }

//    template<>
    void Add(const char* str)
    {
        int idx = 0;

        while(str[idx] != 0x00)
        {
            Add(str[idx]);
            idx++;
        }
        Add((char) 0x00);
    }

//    template<>
    void Add(const char* name, const char* value)
    {
        Add(name);
        Add(value);
    }

//    template<>
    void Add(const std::string& data)
    {
        Add(data.c_str());
    }

//    template<>
    void Add(char* data, size_t size)
    {
        for(size_t i = 0; i < size; i++)
        {
            Add(data[i]);
        }
    }

    const std::vector<char>& Data()
    {
        return m_data;
    }

    void Clear()
    {
        m_data.clear();
    }
};


class TFTPFrameFactory {
public:
    static std::shared_ptr<TFTPFrame> CreateError(ERRORCODE err, const std::string& msg)
    {
        auto frame = std::make_shared<TFTPFrame>();
        frame->Add(static_cast<uint16_t>(OPCODE_ERROR));
        frame->Add(static_cast<uint16_t>(err));
        frame->Add(msg.c_str());
        return frame;
    }

    static std::shared_ptr<TFTPFrame> CreateOptionAck(std::map<std::string, std::string> options)
    {
        auto frame = std::make_shared<TFTPFrame>();
        frame->Add(static_cast<uint16_t>(OPCODE_OPTION_ACK));

        for(auto opt : options)
        {
            frame->Add(opt.first.c_str(), opt.second.c_str());
        }

        return frame;
    }

    static std::shared_ptr<TFTPFrame> CreateDataFrame(uint16_t nblock, const char* data, uint32_t size)
    {
        auto frame = std::make_shared<TFTPFrame>();
        frame->Add(static_cast<uint16_t>(OPCODE_DATA_PACKET));
        frame->Add(nblock);

        for(uint32_t i = 0; i < size; i++)
        {
            frame->Add(data[i]);
        }

        return frame;
    }

    static std::shared_ptr<TFTPFrame> CreateDataAck(uint16_t nblock)
    {
        auto frame = std::make_shared<TFTPFrame>();
        frame->Add(static_cast<uint16_t>(OPCODE_ACK));
        frame->Add(nblock);
        return frame;
    }
};
