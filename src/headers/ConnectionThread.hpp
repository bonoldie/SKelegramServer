#ifndef CONNECTIONTHREAD_H
#define CONNECTIONTHREAD_H

#include "../Includer.hpp"
#define MAXCONNECTIONS 255
typedef void * (*THREADFUNCPTR)(void *);

struct MessagePool
{
    //std::vector<>  
};


class ConnectionThreadPool{
    ConnectionThreadPool() = default;
    ~ConnectionThreadPool() = default;

    void addConnectionThread(int clientSocket);
private:
    void* handleConnection(void *thID);
    static std::string getConnectionIPAndPort(int socket);
private:
    std::array<pthread_t,MAXCONNECTIONS> connectionThreads;
    std::array<int,MAXCONNECTIONS> clientSockets; 

    int thID = 0;
};

#endif