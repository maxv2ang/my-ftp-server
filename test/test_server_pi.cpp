#include "ace/INET_Addr.h"
#include "ace/SOCK_Acceptor.h"
#include "ace/SOCK_Connector.h"
#include "ace/SOCK_Stream.h"
#include "message.h"
#include "server_pi.h"
#include <gtest/gtest.h>
#include <iostream>
#include <thread>

class Server_PI_Fixture: public ::testing::Test
{
protected:
    void SetUp() override
    {
        ACE_INET_Addr port_to_listen(60000);
        acceptor_.open(port_to_listen, 1);
        const char* ip = "127.0.0.1";
        short port = 60000;
        srvr_ = ACE_INET_Addr(port, ip);
    }
    void TearDown() override
    {
        peer_.close();
        acceptor_.close();
    }
    static void SetUpTestCase()
    {
        server_pi_ = new Server_PI();
    }
    static void TearDownTestCase()
    {
        if (server_pi_ != nullptr) {
            delete server_pi_;
        }
    }

public:
    ACE_SOCK_Acceptor acceptor_;
    static Server_PI* server_pi_;
    ACE_SOCK_Connector connector_;
    ACE_SOCK_Stream peer_;
    ACE_INET_Addr srvr_;
};

Server_PI* Server_PI_Fixture::server_pi_ = nullptr;

TEST_F(Server_PI_Fixture, test_not_loged_in)
{
    std::thread t([&]() {
        acceptor_.accept(server_pi_->peer());
        server_pi_->solve_command("PASV\r\n");
        server_pi_->solve_command("QUIT\r\n");
    });
    t.detach();

    char buffer[4096];
    ssize_t bytes_received;

    connector_.connect(peer_, srvr_);

    if ((bytes_received = peer_.recv(buffer, sizeof(buffer))) > 0) {
        buffer[bytes_received] = 0;
        EXPECT_STREQ(MSG_NOT_LOGED_IN, buffer);
    }
    if ((bytes_received = peer_.recv(buffer, sizeof(buffer))) > 0) {
        buffer[bytes_received] = 0;
        EXPECT_STREQ(MSG_GOODBYE, buffer);
    }
}

TEST_F(Server_PI_Fixture, test_solve_user)
{
    std::thread t([&]() {
        acceptor_.accept(server_pi_->peer());
        server_pi_->solve_command("USER nobody\r\n");
        server_pi_->solve_command("USER test\r\n");
    });
    t.detach();

    char buffer[4096];
    ssize_t bytes_received;

    connector_.connect(peer_, srvr_);

    if ((bytes_received = peer_.recv(buffer, sizeof(buffer))) > 0) {
        buffer[bytes_received] = 0;
        EXPECT_STREQ(MSG_USER_FAILED, buffer);
    }
    if ((bytes_received = peer_.recv(buffer, sizeof(buffer))) > 0) {
        buffer[bytes_received] = 0;
        EXPECT_STREQ(MSG_USER_SUCCESS, buffer);
    }
}

TEST_F(Server_PI_Fixture, test_solve_pass)
{
    std::thread t([&]() {
        acceptor_.accept(server_pi_->peer());
        server_pi_->solve_command("PASS nothing\r\n");
        server_pi_->solve_command("PASS test\r\n");
    });
    t.detach();

    char buffer[4096];
    ssize_t bytes_received;

    connector_.connect(peer_, srvr_);

    if ((bytes_received = peer_.recv(buffer, sizeof(buffer))) > 0) {
        buffer[bytes_received] = 0;
        EXPECT_STREQ(MSG_PASS_FAILED, buffer);
    }
    if ((bytes_received = peer_.recv(buffer, sizeof(buffer))) > 0) {
        buffer[bytes_received] = 0;
        EXPECT_STREQ(MSG_PASS_SUCCESS, buffer);
    }
}

TEST_F(Server_PI_Fixture, test_solve_syst)
{
    std::thread t([&]() {
        acceptor_.accept(server_pi_->peer());
        server_pi_->solve_command("SYST\r\n");
    });
    t.detach();

    char buffer[4096];
    ssize_t bytes_received;

    connector_.connect(peer_, srvr_);

    if ((bytes_received = peer_.recv(buffer, sizeof(buffer))) > 0) {
        buffer[bytes_received] = 0;
        EXPECT_STREQ(MSG_SYST_RESPONSE, buffer);
    }
}

TEST_F(Server_PI_Fixture, test_solve_feat)
{
    std::thread t([&]() {
        acceptor_.accept(server_pi_->peer());
        server_pi_->solve_command("FEAT\r\n");
    });
    t.detach();

    char buffer[4096];
    ssize_t bytes_received;

    connector_.connect(peer_, srvr_);

    if ((bytes_received = peer_.recv(buffer, sizeof(buffer))) > 0) {
        buffer[bytes_received] = 0;
        EXPECT_STREQ(MSG_FEAT_RESPONSE, buffer);
    }
}

TEST_F(Server_PI_Fixture, test_solve_type)
{
    std::thread t([&]() {
        acceptor_.accept(server_pi_->peer());
        server_pi_->solve_command("TYPE\r\n");
    });
    t.detach();

    char buffer[4096];
    ssize_t bytes_received;

    connector_.connect(peer_, srvr_);

    if ((bytes_received = peer_.recv(buffer, sizeof(buffer))) > 0) {
        buffer[bytes_received] = 0;
        EXPECT_STREQ(MSG_TYPE_RESPONSE, buffer);
    }
}

TEST_F(Server_PI_Fixture, test_solve_size)
{
    std::thread t([&]() {
        acceptor_.accept(server_pi_->peer());
        server_pi_->solve_command("SIZE badfile\r\n");
    });
    t.detach();

    char buffer[4096];
    ssize_t bytes_received;

    connector_.connect(peer_, srvr_);

    if ((bytes_received = peer_.recv(buffer, sizeof(buffer))) > 0) {
        buffer[bytes_received] = 0;
        EXPECT_STREQ(MSG_SIZE_FAILED, buffer);
    }
}

TEST_F(Server_PI_Fixture, test_solve_pwd)
{
    std::thread t([&]() {
        acceptor_.accept(server_pi_->peer());
        server_pi_->solve_command("PWD\r\n");
    });
    t.detach();

    char buffer[4096];
    ssize_t bytes_received;

    connector_.connect(peer_, srvr_);

    if ((bytes_received = peer_.recv(buffer, sizeof(buffer))) > 0) {
        buffer[bytes_received] = 0;
        // EXPECT_STREQ(MSG_USER_SUCCESS, buffer);
    }
}

TEST_F(Server_PI_Fixture, test_solve_mkd)
{
    std::thread t([&]() {
        acceptor_.accept(server_pi_->peer());
        server_pi_->solve_command("MKD abc\r\n");
        server_pi_->solve_command("MKD abc\r\n");
    });
    t.detach();

    char buffer[4096];
    ssize_t bytes_received;

    connector_.connect(peer_, srvr_);

    if ((bytes_received = peer_.recv(buffer, sizeof(buffer))) > 0) {
        buffer[bytes_received] = 0;
        EXPECT_STREQ(MSG_MKD_SUCCESS, buffer);
    }
    if ((bytes_received = peer_.recv(buffer, sizeof(buffer))) > 0) {
        buffer[bytes_received] = 0;
        EXPECT_STREQ(MSG_MKD_FAILED, buffer);
    }
}

TEST_F(Server_PI_Fixture, test_solve_cwd)
{
    std::thread t([&]() {
        acceptor_.accept(server_pi_->peer());
        server_pi_->solve_command("CWD abc\r\n");
        server_pi_->solve_command("CWD ..\r\n");
        server_pi_->solve_command("CWD badpath\r\n");
    });
    t.detach();

    char buffer[4096];
    ssize_t bytes_received;

    connector_.connect(peer_, srvr_);

    if ((bytes_received = peer_.recv(buffer, sizeof(buffer))) > 0) {
        buffer[bytes_received] = 0;
        EXPECT_STREQ(MSG_CWD_SUCCESS, buffer);
    }
    if ((bytes_received = peer_.recv(buffer, sizeof(buffer))) > 0) {
        buffer[bytes_received] = 0;
        EXPECT_STREQ(MSG_CWD_SUCCESS, buffer);
    }
    if ((bytes_received = peer_.recv(buffer, sizeof(buffer))) > 0) {
        buffer[bytes_received] = 0;
        EXPECT_STREQ(MSG_CWD_FAILED, buffer);
    }
}

TEST_F(Server_PI_Fixture, test_solve_rmd)
{
    std::thread t([&]() {
        acceptor_.accept(server_pi_->peer());
        server_pi_->solve_command("RMD badpath\r\n");
        server_pi_->solve_command("RMD abc\r\n");
    });
    t.detach();

    char buffer[4096];
    ssize_t bytes_received;

    connector_.connect(peer_, srvr_);

    if ((bytes_received = peer_.recv(buffer, sizeof(buffer))) > 0) {
        buffer[bytes_received] = 0;
        EXPECT_STREQ(MSG_RMD_FAILED, buffer);
    }
    if ((bytes_received = peer_.recv(buffer, sizeof(buffer))) > 0) {
        buffer[bytes_received] = 0;
        EXPECT_STREQ(MSG_RMD_SUCCESS, buffer);
    }
}

TEST_F(Server_PI_Fixture, test_solve_dele)
{
    std::thread t([&]() {
        acceptor_.accept(server_pi_->peer());
        server_pi_->solve_command("DELE badfile\r\n");
    });
    t.detach();

    char buffer[4096];
    ssize_t bytes_received;

    connector_.connect(peer_, srvr_);

    if ((bytes_received = peer_.recv(buffer, sizeof(buffer))) > 0) {
        buffer[bytes_received] = 0;
        EXPECT_STREQ(MSG_DELE_FAILED, buffer);
    }
}

TEST_F(Server_PI_Fixture, test_solve_list)
{
    std::thread t([&]() {
        acceptor_.accept(server_pi_->peer());
        server_pi_->solve_command("LIST\r\n");
    });
    t.detach();

    char buffer[4096];
    ssize_t bytes_received;

    connector_.connect(peer_, srvr_);

    if ((bytes_received = peer_.recv(buffer, sizeof(buffer))) > 0) {
        buffer[bytes_received] = 0;
        EXPECT_STREQ(MSG_NOT_IN_PASSIVE, buffer);
    }
}

TEST_F(Server_PI_Fixture, test_solve_retr)
{
    std::thread t([&]() {
        acceptor_.accept(server_pi_->peer());
        server_pi_->solve_command("RETR test.txt\r\n");
    });
    t.detach();

    char buffer[4096];
    ssize_t bytes_received;

    connector_.connect(peer_, srvr_);

    if ((bytes_received = peer_.recv(buffer, sizeof(buffer))) > 0) {
        buffer[bytes_received] = 0;
        EXPECT_STREQ(MSG_NOT_IN_PASSIVE, buffer);
    }
}

TEST_F(Server_PI_Fixture, test_solve_stor)
{
    std::thread t([&]() {
        acceptor_.accept(server_pi_->peer());
        server_pi_->solve_command("STOR test.txt\r\n");
    });
    t.detach();

    char buffer[4096];
    ssize_t bytes_received;

    connector_.connect(peer_, srvr_);

    if ((bytes_received = peer_.recv(buffer, sizeof(buffer))) > 0) {
        buffer[bytes_received] = 0;
        EXPECT_STREQ(MSG_NOT_IN_PASSIVE, buffer);
    }
}

TEST_F(Server_PI_Fixture, test_invalid_command)
{
    std::thread t([&]() {
        acceptor_.accept(server_pi_->peer());
        server_pi_->solve_command("BAD\r\n");
    });
    t.detach();

    char buffer[4096];
    ssize_t bytes_received;

    connector_.connect(peer_, srvr_);

    if ((bytes_received = peer_.recv(buffer, sizeof(buffer))) > 0) {
        buffer[bytes_received] = 0;
        EXPECT_STREQ(MSG_INVALID_COMMAND, buffer);
    }
}

TEST_F(Server_PI_Fixture, test_solve_quit)
{
    std::thread t([&]() {
        acceptor_.accept(server_pi_->peer());
        server_pi_->solve_command("QUIT\r\n");
    });
    t.detach();

    char buffer[4096];
    ssize_t bytes_received;

    connector_.connect(peer_, srvr_);

    if ((bytes_received = peer_.recv(buffer, sizeof(buffer))) > 0) {
        buffer[bytes_received] = 0;
        EXPECT_STREQ(MSG_GOODBYE, buffer);
    }
}
