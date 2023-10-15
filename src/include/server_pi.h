#ifndef SERVER_PI_H
#define SERVER_PI_H

#include "ace/Acceptor.h"
#include "ace/Message_Block.h"
#include "ace/Null_Condition.h"
#include "ace/Null_Mutex.h"
#include "ace/SOCK_Acceptor.h"
#include "ace/SOCK_Stream.h"
#include "ace/Svc_Handler.h"
#include "ace/Synch_Traits.h"
#include "server_dtp.h"

#define MAX_DIR_SIZE 1024

struct Command
{
    std::string command;
    std::string param;
};

class Server_PI: public ACE_Svc_Handler<ACE_SOCK_STREAM, ACE_NULL_SYNCH>
{
    typedef ACE_Svc_Handler<ACE_SOCK_STREAM, ACE_NULL_SYNCH> super;

public:
    /**
     * @brief Construct a new Server_PI object
     *
     */
    Server_PI();
    /**
     * @brief Activate the client handler.  This is typically called by the
     * ACE_Acceptor or ACE_Connector, which passes "this" in as the
     * parameter to open.
     *
     * @return If this method returns -1 the Svc_Handler's close() method
     * is automatically called.
     */
    int open(void* = 0);
    /**
     * @brief Called when input events occur.
     *
     */
    virtual int handle_input(ACE_HANDLE fd = ACE_INVALID_HANDLE);
    /**
     * @brief Perform termination activities on the SVC_HANDLER.
     *
     */
    virtual int handle_close(ACE_HANDLE handle, ACE_Reactor_Mask close_mask);
    /**
     * @brief Parse the command and solve it.
     *
     * @param buf A const string that contains a command.
     */
    void solve_command(const char* buf);
    void solve_user(const Command& command);
    void solve_pass(const Command& command);
    void solve_syst(const Command& command);
    void solve_feat(const Command& command);
    void solve_type(const Command& command);
    void solve_size(const Command& command);
    void solve_pwd(const Command& command);
    void solve_cwd(const Command& command);
    void solve_mkd(const Command& command);
    void solve_rmd(const Command& command);
    void solve_dele(const Command& command);
    void solve_pasv(const Command& command);
    void solve_epsv(const Command& command);
    void solve_list(const Command& command);
    void solve_retr(const Command& command);
    void solve_stor(const Command& command);
    void solve_quit(const Command& command);

private:
    static std::unordered_map<std::string, void (Server_PI::*)(const Command&)>
            handlers_;
    Server_DTP server_dtp_;
    bool is_loged_in_;
    bool is_passive_mod_;
    std::string username_;
    std::string password_;
    std::string current_dir_;
    static std::string root_dir_;
};

#endif // SERVER_PI_H