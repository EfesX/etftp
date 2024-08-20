#pragma once

#include "tftp.hpp"

class OptionsIterator {
private:
    const char* m_data;
    int m_idx;
    int m_len;

    int m_idx_name;
    int m_idx_value;
public:
    OptionsIterator(const char* data, int idx, int len)
    {
        m_data = data;
        m_idx = idx;
        m_len = len;
    }

    const std::pair<const char*, const char*> Option()
    {
        return std::pair<const char*, const char*>(&m_data[m_idx_name], &m_data[m_idx_value]);
    }

    const std::pair<std::string, std::string> Next()
    {
        if(m_idx > m_len) throw std::runtime_error("Options Iterator Error") ;

        m_idx_name = m_idx;

        while(m_data[m_idx] != 0x00) m_idx++;
        m_idx++;

        if(m_idx > m_len) throw std::runtime_error("Options Iterator Error") ;

        m_idx_value = m_idx;

        while(m_data[m_idx] != 0x00) m_idx++;
        m_idx++;

        return { std::string(&m_data[m_idx_name]), std::string(&m_data[m_idx_value]) };
    }

    bool End()
    {
        return m_idx >= m_len;
    }
};

class FrameViewer {
public:
    static OPCODE Opcode(const char* data)
    {
        return static_cast<OPCODE>(data[1]);
    }

    static uint16_t Block(const char* data)
    {
        uint16_t block = static_cast<uint16_t>(static_cast<uint8_t>(data[2]));
        block = block << 8;
        block = block | static_cast<uint16_t>(static_cast<uint8_t>(data[3]));
        return block;
    }

    static const char* SourceFile(const char* data)
    {
        return &data[2];
    }

    static const char* DestinationFile(const char* data)
    {
        return FrameViewer::SourceFile(data);
    }

    static const char* Type(const char* data)
    {
        int idx = 2;
        idx += std::string(SourceFile(data)).length() + 1;
        return &data[idx];
    }

    static OptionsIterator* OptionsBegin(const char* data, int len)
    {
        int idx = 2;
        idx += std::string(SourceFile(data)).length() + 1;
        idx += std::string(Type(data)).length() + 1;

        return new OptionsIterator(data, idx, len);
    }

    static const char* Data(const char* data)
    {
        return &data[4];
    }
    

};
