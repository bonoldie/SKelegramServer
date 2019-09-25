#ifndef CONNECTIONTHREAD_H
#define CONNECTIONTHREAD_H

#include "../Includer.hpp"

#define MAXCONNECTIONS 255

struct ConnectionData
{
    int clientSocket;
    bool messageAvailable;
    pthread_t owner;
    std::string incomingMessage;
    std::vector<std::string> toSendBuffer;
};

void *handleConnection(void *connectionData);
void *broadcastRoutine(void *connectionsData);

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
    pthread_t broadcastThread;

    int threadCounter = 0;
    int index = 0;
};

#endif