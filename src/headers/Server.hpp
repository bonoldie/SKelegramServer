#ifndef SERVER_H
#define SERVER_H

#include "../Includer.hpp"
#include "./ConnectionThread.hpp"

#define DEFAULT_PORT 45678

typedef struct sockaddr_in SOCKETADDRIN;

void *broadcastRoutine(void *connectionThreadPool);
void *chatRoutine(void *connectionThreadPool);

class Server {
public:
    Server() : port(DEFAULT_PORT){};
    Server(int _port): port(_port){};
    ~Server() = default;

    void initialize();
    int bindAndListen();
    void startAccept();

    ConnectionThreadPool *threadPool;
private:
    int serverSocketFD;
    SOCKETADDRIN listeningAddress;
    int port; 
    
    pthread_t broadcastThread;
};

#endif