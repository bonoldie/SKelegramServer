#ifndef SERVER_H
#define SERVER_H

#include "../Includer.hpp"

#define DEFAULT_PORT 45678

typedef struct sockaddr_in SOCKETADDRIN;

class Server {
public:
    Server() : port(DEFAULT_PORT){};
    Server(int _port): port(_port){};
    ~Server() = default;

    void initialize();
    int bindAndListen(int *socketFD, SOCKETADDRIN *address);

private:
    int port; 
};

#endif