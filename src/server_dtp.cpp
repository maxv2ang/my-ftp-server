#include "server_dtp.h"

#include "ace/FILE_Connector.h"
#include "ace/File_Lock.h"
#include "ace/Log_Msg.h"
#include "ace/OS.h"
#include "ace/OS_NS_sys_stat.h"
#include "ace/OS_NS_unistd.h"
#include "ace/SOCK_Acceptor.h"
#include "ace/SOCK_Stream.h"
#include "utils.h"
#include <string>
#include <sys/file.h>
#include <sys/sendfile.h>
#include <vector>

int Server_DTP::open()
{
    int rand_number = 0;
    // initialize acceptor by using open(), listen to a random port.
    while (1) {
        rand_number = rand_port();
        port_to_listen_.set_port_number(rand_number);
        if (acceptor_.open(port_to_listen_, 1) == -1) {
            ACE_DEBUG((LM_DEBUG,
                       ACE_TEXT("acceptor.open: port %d has been occupied.\n"),
                       rand_number));
        } else {
            ACE_DEBUG((LM_DEBUG,
                       ACE_TEXT("acceptor.open: listened to port %d.\n"),
                       rand_number));
            break;
        }
    }
    return rand_number;
}

int Server_DTP::accept()
{
    while (1) {
        // waiting for client to connect
        if (acceptor_.accept(peer_, &peer_addr_) == -1) {
            ACE_DEBUG((LM_DEBUG, ACE_TEXT("accept error!\n")));
        } else {
            ACE_TCHAR peer_name[MAXHOSTNAMELEN];
            peer_addr_.addr_to_string(peer_name, MAXHOSTNAMELEN);
            ACE_DEBUG((LM_DEBUG, ACE_TEXT("Connection from %s\n"), peer_name));
            break;
        }
    }
    return 0;
}
void Server_DTP::close()
{
    peer_.close();
    acceptor_.close();
}

int Server_DTP::send_file(std::string path)
{
    ACE_FILE_Connector connector;
    connector.connect(file_, ACE_FILE_Addr(path.c_str()), 0, ACE_Addr::sap_any,
                      0, O_RDONLY, ACE_DEFAULT_FILE_PERMS);

    ACE_File_Lock file_lock(file_.get_handle());
    file_lock.acquire_read();

    int send_count = 0;
    long offset = 0;
    while (1) {
        send_count = sendfile(peer_.get_handle(), file_.get_handle(), &offset,
                              MAX_BUFFER_SIZE);

        if (send_count < 0) {
            if (errno == EAGAIN || errno == EINTR || errno == EWOULDBLOCK) {
                continue;
            } else {
                file_lock.release();
                this->close();
                return -1;
            }
        } else if (send_count == 0) {
            file_lock.release();
            this->close();
            return 0;
        }
    }
    file_lock.release();
    this->close();
    return 0;
}

int Server_DTP::resv_file(std::string path)
{
    ACE_FILE_Connector connector;
    connector.connect(file_, ACE_FILE_Addr(path.c_str()), 0, ACE_Addr::sap_any,
                      0, O_RDWR | O_CREAT | O_TRUNC, ACE_DEFAULT_FILE_PERMS);

    ACE_File_Lock file_lock(file_.get_handle());
    file_lock.acquire_write();

    int recv_count = 0;
    while (1) {
        recv_count = peer_.recv(buf_, MAX_BUFFER_SIZE);
        if (recv_count < 0) {
            if (errno == EAGAIN || errno == EINTR || errno == EWOULDBLOCK) {
                continue;
            } else {
                file_lock.release();
                this->close();
                return -1;
            }
        } else if (recv_count == 0) {
            file_lock.release();
            this->close();
            return 0;
        } else {
            int write_count = file_.send(buf_, recv_count);
            if (write_count <= 0) {
                file_lock.release();
                this->close();
                return -1;
            }
        }
    }
    file_lock.release();
    this->close();
    return 0;
}

int Server_DTP::solve_list_data(const std::string path)
{
    std::vector<std::string> list;
    ACE_DEBUG((LM_DEBUG, ACE_TEXT("START SEND!\n")));
    get_ls_data(path.c_str(), list);
    for (auto s : list) {
        peer_.send_n(s.c_str(), s.length());
    }
    ACE_DEBUG((LM_DEBUG, ACE_TEXT("SEND FINISHED!\n")));
    this->close();
    return 0;
}

Server_DTP::~Server_DTP()
{
    peer_.close();
    acceptor_.close();
}
