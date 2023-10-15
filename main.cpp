#include "ace/Acceptor.h"
#include "ace/INET_Addr.h"
#include "ace/Reactor.h"
#include "ace/SOCK_Acceptor.h"
#include "server_pi.h"

typedef ACE_Acceptor<Server_PI, ACE_SOCK_ACCEPTOR> ClientAcceptor;

int ACE_TMAIN(int, ACE_TCHAR*[])
{
    ACE_INET_Addr port_to_listen(3000);
    ClientAcceptor acceptor;
    if (acceptor.open(port_to_listen, ACE_Reactor::instance(), ACE_NONBLOCK) ==
        -1) {
        return -1;
    }

    ACE_Reactor::instance()->run_reactor_event_loop();
    return 0;
}
