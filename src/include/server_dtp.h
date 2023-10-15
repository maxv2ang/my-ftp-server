#ifndef SERVER_DTP_H
#define SERVER_DTP_H

#include "ace/FILE_IO.h"
#include "ace/SOCK_Acceptor.h"
#include "ace/SOCK_Stream.h"
#include <string>

#define MAX_BUFFER_SIZE 5700

class Server_DTP
{
public:
    /**
     * @brief Destroy the Server_DTP object
     *
     */
    ~Server_DTP();
    /**
     * @brief Initialize acceptor by using open(), listen to a random port.
     *
     * @return the port number
     */
    int open();
    /**
     * @brief Waiting for client to connect.
     *
     * @return return 0 when connection established.
     */
    int accept();
    /**
     * @brief Close down and release all resources.
     *
     */
    void close();
    /**
     * @brief Send the list data to client.
     *
     * @param path the file or directory path
     * @return return 0 when succeed, else return -1;
     */
    int solve_list_data(std::string path);
    /**
     * @brief Send file to client.
     *
     * @param path the file path
     * @return return 0 when succeed, else return -1;
     */
    int send_file(std::string path);
    /**
     * @brief Receive file from client.
     *
     * @param path the file path
     * @return return 0 when succeed, else return -1;
     */
    int resv_file(std::string path);

private:
    ACE_SOCK_Acceptor acceptor_;
    ACE_INET_Addr port_to_listen_;
    ACE_SOCK_Stream peer_;
    ACE_INET_Addr peer_addr_;
    ACE_FILE_IO file_;
    char buf_[MAX_BUFFER_SIZE];
};

#endif // SERVER_DTP_H