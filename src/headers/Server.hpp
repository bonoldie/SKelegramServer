#ifndef SERVER_H
#define SERVER_H

#include "../Includer.hpp"
#include "./ConnectionThread.hpp"
#include "./SKelegramCore.hpp"

#define DEFAULT_PORT 45678

typedef struct sockaddr_in SOCKETADDRIN;

struct ServerRoutineData {
    ConnectionThreadPool *cThreadPool;
    SKelegramCore *skelegramCore;
};

void *serverRoutine(void *data);

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
    SKelegramCore *skelegramCore;
    int port;
    pthread_t serverRoutineThread;
};

#endif