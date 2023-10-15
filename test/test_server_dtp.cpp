#include "ace/SOCK_Acceptor.h"
#include "ace/SOCK_Connector.h"
#include "ace/SOCK_Stream.h"
#include "server_dtp.h"
#include <gtest/gtest.h>
#include <thread>

class Server_DTP_Fixture: public ::testing::Test
{
protected:
    void SetUp() override
    {
        port_ = server_dtp_.open();
        const char* ip = "127.0.0.1";
        srvr_ = ACE_INET_Addr(port_, ip);
    }
    void TearDown() override
    {
        peer_.close();
        server_dtp_.close();
    }

public:
    Server_DTP server_dtp_;
    ACE_SOCK_Connector connector_;
    ACE_SOCK_Stream peer_;
    ACE_INET_Addr srvr_;
    int port_;
};

TEST_F(Server_DTP_Fixture, test_solve_list_data)
{
    std::thread t([&]() {
        server_dtp_.accept();
    });
    t.detach();

    char buffer[4096];
    ssize_t bytes_received;

    connector_.connect(peer_, srvr_);
    EXPECT_EQ(0, server_dtp_.solve_list_data(std::string(".")));

    if ((bytes_received = peer_.recv(buffer, sizeof(buffer))) > 0) {
        buffer[bytes_received] = 0;
    }
}
