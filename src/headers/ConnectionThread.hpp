#ifndef CONNECTIONTHREAD_H
#define CONNECTIONTHREAD_H

#include "../Includer.hpp"
#define MAXCONNECTIONS 255
typedef void * (*THREADFUNCPTR)(void *);


class ConnectionThreadPool{
public:
    ConnectionThreadPool();
    ~ConnectionThreadPool() = default;

    void addConnectionThread(int clientSocket);
    void addMessageToThreads(std::string message);
    int thID = 0;
private:
    void* handleConnection(void *thID);
    static std::string getConnectionIPAndPort(int socket);
private:
    std::array<pthread_t,MAXCONNECTIONS> connectionThreads;
    std::array<int,MAXCONNECTIONS> clientSockets; 
    std::array<std::vector<std::string>,MAXCONNECTIONS> threadMessages;

    std::array<int,MAXCONNECTIONS> IDs;

    
};

#endif