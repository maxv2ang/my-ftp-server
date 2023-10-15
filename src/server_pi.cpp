#include "server_pi.h"

#include "ace/Acceptor.h"
#include "ace/INET_Addr.h"
#include "ace/Log_Msg.h"
#include "ace/OS.h"
#include "ace/OS_NS_sys_stat.h"
#include "ace/OS_NS_unistd.h"
#include "ace/Reactor.h"
#include "ace/SOCK_Acceptor.h"
#include "message.h"
#include "utils.h"
#include <cstdlib>
#include <iostream>

std::string Server_PI::root_dir_ = "ftp";

std::unordered_map<std::string, std::string> user_list = {
        {"test", "test"},
        {"wq", "123456"},
};

std::unordered_map<std::string, void (Server_PI::*)(const Command&)>
        Server_PI::handlers_ = {
                {"USER", &Server_PI::solve_user},
                {"PASS", &Server_PI::solve_pass},
                {"SYST", &Server_PI::solve_syst},
                {"FEAT", &Server_PI::solve_feat},
                {"TYPE", &Server_PI::solve_type},
                {"SIZE", &Server_PI::solve_size},
                {"PWD", &Server_PI::solve_pwd},
                {"CWD", &Server_PI::solve_cwd},
                {"MKD", &Server_PI::solve_mkd},
                {"RMD", &Server_PI::solve_rmd},
                {"DELE", &Server_PI::solve_dele},
                {"PASV", &Server_PI::solve_pasv},
                {"EPSV", &Server_PI::solve_epsv},
                {"LIST", &Server_PI::solve_list},
                {"RETR", &Server_PI::solve_retr},
                {"STOR", &Server_PI::solve_stor},
                {"QUIT", &Server_PI::solve_quit},
};

Command parse_command(const char* buf)
{
    Command command;
    while (*buf != ' ' && *buf != '\r') {
        command.command.push_back(*buf);
        ++buf;
    }
    if (*buf == ' ') {
        ++buf;
    }
    while (*buf != '\r') {
        command.param.push_back(*buf);
        ++buf;
    }
    return command;
}

Server_PI::Server_PI()
{
    this->is_loged_in_ = false;
    this->is_passive_mod_ = false;
    if (!is_dir_exist(root_dir_)) {
        mkdir(root_dir_.c_str(), 0755);
    }
    root_dir_ = get_realpath(root_dir_);
    current_dir_ = root_dir_;
    ACE_DEBUG((LM_DEBUG, ACE_TEXT("root: %s\n"), root_dir_.c_str()));
    ACE_DEBUG((LM_DEBUG, ACE_TEXT("current: %s\n"), current_dir_.c_str()));
}

int Server_PI::open(void* p)
{
    if (super::open(p) == -1) {
        return -1;
    }

    ACE_TCHAR peer_name[MAXHOSTNAMELEN];
    ACE_INET_Addr peer_addr;
    if (this->peer().get_remote_addr(peer_addr) == 0 &&
        peer_addr.addr_to_string(peer_name, MAXHOSTNAMELEN) == 0) {
        ACE_DEBUG((LM_DEBUG, ACE_TEXT("(%P|%t) Connection from %s\n"),
                   peer_name));
    }

    this->peer().send_n("220 ok\r\n", 8);
    return 0;
}

int Server_PI::handle_input(ACE_HANDLE)
{
    const size_t INPUT_SIZE = 4096;
    char buffer[INPUT_SIZE] = {0};
    ssize_t recv_cnt, send_cnt;

    recv_cnt = this->peer().recv(buffer, sizeof(buffer));
    if (recv_cnt <= 0) {
        this->is_loged_in_ = false;
        ACE_DEBUG((LM_DEBUG, ACE_TEXT("(%P|%t) Connection closed\n")));
        return -1;
    }
    ACE_DEBUG((LM_DEBUG, ACE_TEXT("recv data %s"), buffer));

    solve_command(buffer);

    return 0;
}

int Server_PI::handle_close(ACE_HANDLE h, ACE_Reactor_Mask mask)
{
    return super::handle_close(h, mask);
}

void Server_PI::solve_command(const char* buf)
{
    Command command = parse_command(buf);
    if (this->is_loged_in_ == false) {
        if (command.command == "QUIT") {
            this->peer().send_n(MSG_GOODBYE, strlen(MSG_GOODBYE));
            return;
        } else if (command.command != "USER" && command.command != "PASS") {
            this->peer().send_n(MSG_NOT_LOGED_IN, strlen(MSG_NOT_LOGED_IN));
            return;
        }
    }
    auto iter = this->handlers_.find(command.command);
    if (iter != this->handlers_.end()) {
        (this->*(iter->second))(command);
    } else {
        this->peer().send_n(MSG_INVALID_COMMAND, strlen(MSG_INVALID_COMMAND));
    }
}

void Server_PI::solve_user(const Command& command)
{
    this->is_loged_in_ = false;
    if (user_list.count(command.param) > 0) {
        this->username_ = command.param;
        this->peer().send_n(MSG_USER_SUCCESS, strlen(MSG_USER_SUCCESS));
    } else {
        this->username_ = "";
        this->peer().send_n(MSG_USER_FAILED, strlen(MSG_USER_FAILED));
    }
}

void Server_PI::solve_pass(const Command& command)
{
    if (user_list.count(username_) > 0 &&
        user_list[this->username_] == command.param) {
        this->password_ = command.param;
        this->is_loged_in_ = true;
        this->peer().send_n(MSG_PASS_SUCCESS, strlen(MSG_PASS_SUCCESS));
    } else {
        this->password_ = "";
        this->is_loged_in_ = false;
        current_dir_ = root_dir_;
        this->peer().send_n(MSG_PASS_FAILED, strlen(MSG_PASS_FAILED));
    }
}

void Server_PI::solve_syst(const Command& command)
{
    this->peer().send_n(MSG_SYST_RESPONSE, strlen(MSG_SYST_RESPONSE));
}

void Server_PI::solve_feat(const Command& command)
{
    this->peer().send_n(MSG_FEAT_RESPONSE, strlen(MSG_FEAT_RESPONSE));
}

void Server_PI::solve_type(const Command& command)
{
    this->peer().send_n(MSG_TYPE_RESPONSE, strlen(MSG_TYPE_RESPONSE));
}

void Server_PI::solve_size(const Command& command)
{
    size_t size = 0;
    char msg[1024] = {0};
    if (get_size(command.param, size)) {
        ACE_OS::snprintf(msg, sizeof(msg), "213 %lu\r\n", size);
        this->peer().send_n(msg, strlen(msg));
    } else {
        this->peer().send_n(MSG_SIZE_FAILED, strlen(MSG_SIZE_FAILED));
    }
}

void Server_PI::solve_pwd(const Command& command)
{
    char msg[1024] = {0};
    ACE_OS::snprintf(msg, sizeof(msg),
                     "257 \"%s\" is the current directory\r\n",
                     this->current_dir_.c_str());
    this->peer().send_n(msg, ACE_OS::strlen(msg));
}

void Server_PI::solve_cwd(const Command& command)
{
    std::string tmp = "";
    if (command.param[0] == '/') {
        tmp = root_dir_ + command.param;
    } else {
        tmp = current_dir_ + "/" + command.param;
    }
    tmp = get_realpath(tmp);
    if (tmp != "" && startsWith(tmp, root_dir_) && is_dir_exist(tmp)) {
        current_dir_ = tmp;
        this->peer().send_n(MSG_CWD_SUCCESS, strlen(MSG_CWD_SUCCESS));
    } else {
        this->peer().send_n(MSG_CWD_FAILED, strlen(MSG_CWD_FAILED));
    }
}

void Server_PI::solve_mkd(const Command& command)
{
    std::string tmp = "";
    if (command.param[0] == '/') {
        tmp = root_dir_ + command.param;
    } else {
        tmp = current_dir_ + "/" + command.param;
    }
    tmp = get_realpath(tmp);
    if (tmp != "" && startsWith(tmp, root_dir_) && !is_dir_exist(tmp) &&
        mkdir(tmp.c_str(), 0755) == 0) {
        this->peer().send_n(MSG_MKD_SUCCESS, strlen(MSG_MKD_SUCCESS));
    } else {
        this->peer().send_n(MSG_MKD_FAILED, strlen(MSG_MKD_FAILED));
    }
}

void Server_PI::solve_rmd(const Command& command)
{
    std::string tmp = "";
    if (command.param[0] == '/') {
        tmp = root_dir_ + command.param;
    } else {
        tmp = current_dir_ + "/" + command.param;
    }
    tmp = get_realpath(tmp);
    if (tmp != "" && startsWith(tmp, root_dir_) &&
        ACE_OS::rmdir(tmp.c_str()) == 0) {
        this->peer().send_n(MSG_RMD_SUCCESS, strlen(MSG_RMD_SUCCESS));
    } else {
        this->peer().send_n(MSG_RMD_FAILED, strlen(MSG_RMD_FAILED));
    }
}

void Server_PI::solve_dele(const Command& command)
{
    std::string tmp = "";
    if (command.param[0] == '/') {
        tmp = root_dir_ + command.param;
    } else {
        tmp = current_dir_ + "/" + command.param;
    }
    tmp = get_realpath(tmp);
    if (tmp != "" && startsWith(tmp, root_dir_) &&
        ACE_OS::unlink(tmp.c_str()) == 0) {
        this->peer().send_n(MSG_DELE_SUCCESS, strlen(MSG_DELE_SUCCESS));
    } else {
        this->peer().send_n(MSG_DELE_FAILED, strlen(MSG_DELE_FAILED));
    }
}

void Server_PI::solve_pasv(const Command& command)
{
    int port = this->server_dtp_.open();
    char msg[1024] = {0};
    ACE_OS::snprintf(msg, sizeof(msg),
                     "227 Entering Passive Mode (%u,%u,%u,%u,%u,%u).\r\n", 127,
                     0, 0, 1, port >> 8, port & 0xFF);
    this->peer().send_n(msg, ACE_OS::strlen(msg));
    this->server_dtp_.accept();
    this->is_passive_mod_ = true;
}

void Server_PI::solve_epsv(const Command& command)
{
    int port = this->server_dtp_.open();
    char msg[1024] = {0};
    ACE_OS::snprintf(msg, sizeof(msg),
                     "229 Entering Extended Passive Mode (|||%d|)\r\n", port);
    this->peer().send_n(msg, ACE_OS::strlen(msg));
    this->server_dtp_.accept();
    this->is_passive_mod_ = true;
}

void Server_PI::solve_list(const Command& command)
{
    if (is_passive_mod_ == false) {
        this->peer().send_n(MSG_NOT_IN_PASSIVE, strlen(MSG_NOT_IN_PASSIVE));
        return;
    }

    std::string tmp = "";
    if (command.param == "") {
        tmp = current_dir_;
    } else if (command.param[0] == '/') {
        tmp = root_dir_ + command.param;
    } else {
        tmp = current_dir_ + "/" + command.param;
    }
    this->peer().send_n(MSG_LIST_START, strlen(MSG_LIST_START));
    if (this->server_dtp_.solve_list_data(tmp) == 0) {
        this->peer().send_n(MSG_LIST_FINISHED, strlen(MSG_LIST_FINISHED));
    }
    this->is_passive_mod_ = false;
}

void Server_PI::solve_retr(const Command& command)
{
    if (is_passive_mod_ == false) {
        this->peer().send_n(MSG_NOT_IN_PASSIVE, strlen(MSG_NOT_IN_PASSIVE));
        return;
    }

    char msg[1024] = {0};
    size_t size = 0;
    std::string tmp = "";
    if (command.param[0] == '/') {
        tmp = root_dir_ + command.param;
    } else {
        tmp = current_dir_ + "/" + command.param;
    }
    std::cout << tmp << std::endl;
    if (get_size(tmp, size)) {
        ACE_OS::snprintf(
                msg, sizeof(msg),
                "150 Opening BINARY mode data connection for %s (%lu bytes).\r\n",
                command.param.c_str(), size);
        this->peer().send_n(msg, ACE_OS::strlen(msg));
        if (this->server_dtp_.send_file(tmp) == 0) {
            this->peer().send_n(MSG_TRANSFER_COMPLETE,
                                strlen(MSG_TRANSFER_COMPLETE));
        } else {
            this->peer().send_n(MSG_RETR_FAILED, strlen(MSG_RETR_FAILED));
        }
    } else {
        this->peer().send_n(MSG_RETR_FAILED, strlen(MSG_RETR_FAILED));
        this->server_dtp_.close();
    }

    this->is_passive_mod_ = false;
}

void Server_PI::solve_stor(const Command& command)
{
    if (is_passive_mod_ == false) {
        this->peer().send_n(MSG_NOT_IN_PASSIVE, strlen(MSG_NOT_IN_PASSIVE));
        return;
    }

    this->peer().send_n(MSG_STOR_START, strlen(MSG_STOR_START));
    std::string tmp = "";
    if (command.param[0] == '/') {
        tmp = root_dir_ + command.param;
    } else {
        tmp = current_dir_ + "/" + command.param;
    }
    if (this->server_dtp_.resv_file(tmp) == 0) {
        this->peer().send_n(MSG_TRANSFER_COMPLETE,
                            strlen(MSG_TRANSFER_COMPLETE));
    }

    this->is_passive_mod_ = false;
}

void Server_PI::solve_quit(const Command& command)
{
    this->peer().send_n(MSG_GOODBYE, strlen(MSG_GOODBYE));
}
