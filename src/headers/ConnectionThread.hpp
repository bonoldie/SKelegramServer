#ifndef CONNECTIONTHREAD_H
#define CONNECTIONTHREAD_H

#include "../Includer.hpp"

#define MAXCONNECTIONS 255

struct ConnectionData
{
    int clientSocket;
    bool messageAvailable;
    std::vector<std::string> toSendBuffer;
    std::array<std::string,1> incomingMessages;
};

void *handleConnection(void *connectionData);

class ConnectionThreadPool
{
public:
    ConnectionThreadPool();
    ~ConnectionThreadPool() = default;

    void addConnectionThread(int clientSocket);
    void broadcastMessage(std::string message);

    static std::string getConnectionIPAndPort(int socket);

    std::vector<ConnectionData> connectionsData;
private:
    std::array<pthread_t, MAXCONNECTIONS> threads;

    int threadCounter = 0;
    int index = 0;
};

#endif