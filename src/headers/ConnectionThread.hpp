#ifndef CONNECTIONTHREAD_H
#define CONNECTIONTHREAD_H

#include "../Includer.hpp"

#define MAXCONNECTIONS 255

struct ConnectionData
{
    int clientSocket;
    std::vector<std::string> toSendBuffer;
    std::vector<std::string> receivedBuffer;
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

    void clearRecData();

    std::vector<ConnectionData> connectionsData;
private:
    std::array<pthread_t, MAXCONNECTIONS> threads;

    int threadCounter = 0;
    int index = 0;
};

#endif