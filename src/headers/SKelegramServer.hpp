#ifndef SERVER_H
#define SERVER_H

#include "../Includer.hpp"
#include "./SKelegramCore.hpp"

#define DEFAULT_PORT 45678

typedef struct sockaddr_in SOCKETADDRIN;

struct ServerRoutineData
{
    ConnectionPool *connectionPool;
    SKelegramCore *skelegramCore;
};

class SKelegramServer
{
public:
    SKelegramServer() : port(DEFAULT_PORT){};
    SKelegramServer(int _port) : port(_port){};
    ~SKelegramServer() = default;

    // Initialize the server logger and the main thread pool
    // Future updates will implement more pools
    void initialize();

    // Try to bind and listen to the selected port or if not set !! to the default one DEFAULT_PORT
    int bindAndListen();

    // Start to accept request of connetion from the binded socket
    // It's blocking until any request is rised
    void startAccept();

    ConnectionPool *connectionPool;

private:
    int serverSocketFD;
    SOCKETADDRIN listeningAddress;
    SKelegramCore *skelegramCore;
    int port;
    pthread_t serverRoutineThread;
};

#endif