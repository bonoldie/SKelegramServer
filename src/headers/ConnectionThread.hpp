#ifndef CONNECTIONTHREAD_H
#define CONNECTIONTHREAD_H

#include "../Includer.hpp"

#define MAXCONNECTIONS 255

struct ConnectionData
{
    int clientSocket;
    std::vector<std::string> *incomingMessages;
};

struct BroadcastData
{
    std::vector<std::string> *incomingMessagges;
    std::vector<int> *sockets;
};

void *handleConnection(void *connectionData);
void *broadcastRoutine(void *threadData);

void *receiveRoutine(void *threadData);
void *sendRoutine(void *threadData);

void *testRoutine(void *threadData);

class ConnectionThreadPool
{
public:
    ConnectionThreadPool();
    ~ConnectionThreadPool() = default;

    void addConnectionThread(int clientSocket);
    void broadcastMessage(std::string message);

    static std::string getConnectionIPAndPort(int socket);

    std::vector<std::string> incomingMessagges;
    std::vector<int> sockets;
    std::vector<pthread_t> threads;

private:
    pthread_t broadcastThread;

    int connectionsCounter = 0;
};

#endif