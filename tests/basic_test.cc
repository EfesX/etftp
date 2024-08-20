#include <gtest/gtest.h>

#include "client.hpp"
#include "frame.hpp"
#include "frame_viewer.hpp"

TEST(TFTPTests, ClientTest)
{
    Client client;
    EXPECT_EQ(sizeof(client.sockaddr), sizeof(sockaddr_in));
    EXPECT_EQ(sizeof(client.sock), sizeof(SOCKET));
    EXPECT_EQ(sizeof(client.blksize), sizeof(unsigned long));
    EXPECT_EQ(sizeof(client.tsize), sizeof(unsigned long));
    EXPECT_EQ(sizeof(client.timeout), sizeof(unsigned long));
    EXPECT_EQ(sizeof(client.msgin->addrin), sizeof(sockaddr_in));
    EXPECT_EQ(sizeof(client.msgin->len), sizeof(int));
    client.msgin = std::make_shared<IncommingMessage>();
    client.msgin->data = (char*) malloc(sizeof("TEST"));
    memcpy(client.msgin->data, "TEST", sizeof("TEST"));
    EXPECT_TRUE(memcmp(client.msgin->data, "TEST", sizeof("TEST")) == 0);
}

TEST(TFTPTests, FrameViewerTest)
{
    TFTPFrame frame;
    frame.Add(static_cast<uint16_t>(OPCODE_READ_REQUEST));
    frame.Add(std::string("file", sizeof("file")));
    frame.Add(std::string("octet", sizeof("octet")));
    frame.Add("K1", "V1");
    frame.Add("K2", "V2");
    frame.Add("K3", "V3");
    EXPECT_EQ(FrameViewer::Opcode(frame.Data().data()), OPCODE_READ_REQUEST);
    EXPECT_STREQ(FrameViewer::Type(frame.Data().data()), "octet");
    EXPECT_STREQ(FrameViewer::SourceFile(frame.Data().data()), "file");
    EXPECT_STREQ(FrameViewer::DestinationFile(frame.Data().data()), "file");
    auto it = FrameViewer::OptionsBegin(frame.Data().data(), frame.Data().size());
    EXPECT_FALSE(it->End());
    {
        auto option = it->Next();
        EXPECT_STREQ(option.first.c_str(), "K1");
        EXPECT_STREQ(option.second.c_str(), "V1");
    }
    {
        auto option = it->Next();
        EXPECT_STREQ(option.first.c_str(), "K2");
        EXPECT_STREQ(option.second.c_str(), "V2");
    }
    {
        auto option = it->Next();
        EXPECT_STREQ(option.first.c_str(), "K3");
        EXPECT_STREQ(option.second.c_str(), "V3");
    }
    EXPECT_TRUE(it->End());
}

TEST(TFTPTests, FrameFactoryTest)
{
    {
        auto err = TFTPFrameFactory::CreateError(ERR_DISK_FULL_OR_ALLOC_EXCEEDED, "Test Msg");
        EXPECT_EQ(FrameViewer::Opcode(err->Data().data()), OPCODE_ERROR);
        EXPECT_EQ(err->Data().data()[2], 0x00);
        EXPECT_EQ(err->Data().data()[3], ERR_DISK_FULL_OR_ALLOC_EXCEEDED);
        EXPECT_STREQ(&err->Data().data()[4], "Test Msg");
    }
    {
        auto frame = TFTPFrameFactory::CreateDataAck(5);
        EXPECT_EQ(FrameViewer::Opcode(frame->Data().data()), OPCODE_ACK);
        EXPECT_EQ(FrameViewer::Block(frame->Data().data()), 5);
    }
    {
        auto frame = TFTPFrameFactory::CreateDataFrame(5, "TEST", sizeof("TEST"));
        EXPECT_EQ(FrameViewer::Opcode(frame->Data().data()), OPCODE_DATA_PACKET);
        EXPECT_EQ(FrameViewer::Block(frame->Data().data()), 5);
        EXPECT_STREQ(std::string(FrameViewer::Data(frame->Data().data())).c_str(), "TEST");
    }
    {
        std::map<std::string, std::string> opts = {
            {"K1", "V1"},
            {"K2", "V2"},
            {"K3", "V3"},
            {"K4", "V4"},
        };
        auto frame = TFTPFrameFactory::CreateOptionAck(opts);
        EXPECT_EQ(FrameViewer::Opcode(frame->Data().data()), OPCODE_OPTION_ACK);
        int idx = 2;
        EXPECT_STREQ(&frame->Data().data()[idx], "K1"); idx += 3;
        EXPECT_STREQ(&frame->Data().data()[idx], "V1"); idx += 3;
        EXPECT_STREQ(&frame->Data().data()[idx], "K2"); idx += 3;
        EXPECT_STREQ(&frame->Data().data()[idx], "V2"); idx += 3;
        EXPECT_STREQ(&frame->Data().data()[idx], "K3"); idx += 3;
        EXPECT_STREQ(&frame->Data().data()[idx], "V3"); idx += 3;
        EXPECT_STREQ(&frame->Data().data()[idx], "K4"); idx += 3;
        EXPECT_STREQ(&frame->Data().data()[idx], "V4");;
    }
}
