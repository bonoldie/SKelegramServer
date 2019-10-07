#include "./headers/SKelegramCore.hpp"
#define WELCOME_MESSAGE " ################################# \n # Welcome in SKelegram chat :) # \n ################################# \n"

uint ConnectionPool::poolCounter = 0;

std::mutex addSocketMutex;

ConnectionPool::ConnectionPool() : poolID(poolCounter++)
{
    pthread_t poolThread;
    SKelegramConnectionPoolData poolData;

    poolData.clientSockets = &registeredSockets;
    poolData.rawData = &rawData;
    *poolData.isReady = 0;

    pthread_create(&poolThread, NULL, poolReceiveRoutine, (void *)&poolData);

    while (*poolData.isReady == 0);
}

void ConnectionPool::addReceiver(int clientSocket)
{
    ML::log_info(std::string("Client connected from ") + ConnectionPool::getConnectionIPAndPort(clientSocket));

    // Using mutex for operate with shared data
    addSocketMutex.lock();

    registeredSockets.push_back(clientSocket);

    addSocketMutex.unlock();

    send(clientSocket, &WELCOME_MESSAGE, sizeof(WELCOME_MESSAGE), 0);
}

void *poolReceiveRoutine(void *threadData)
{
    SKelegramConnectionPoolData connectionData = *(SKelegramConnectionPoolData *)threadData;

    *connectionData.isReady = 1;

    std::string receivedString;

    int receivedFlag;

    while (1)
    {   
        for (int currentSocket : *connectionData.clientSockets)
        {
            receivedString.clear();

            char buffer[1];

            while (receivedString.find("&(end)&") == std::string::npos && (receivedFlag = recv(currentSocket, &buffer, 1, 0)) > 0)
            {
                receivedString += buffer[0];
            }

            // if receivedFlag is 0 -> connection is close
            // otherwise if receivedString contains end delimiter it's added to the raw data queue
            if (receivedFlag == 0)
            {
                receivedString = "&(server)&CLOSECONNECTION&(end)&";
            }
            else if (receivedString.find("&(end)&") != std::string::npos)
            {
                SKelegramRawData receivedRawData;

                receivedRawData.rawData = receivedString;
                receivedRawData.clientSocket = currentSocket;

                connectionData.rawData->push_back(receivedRawData);
            }
        }
    }
}

void ConnectionPool::broadcastData(std::string rawData)
{
    for (int clientSocket : registeredSockets)
    {
        send(clientSocket, rawData.c_str(), strlen(rawData.c_str()), 0);
    }
}

std::string ConnectionPool::getConnectionIPAndPort(int socket)
{
    socklen_t len;
    struct sockaddr_storage address;
    char IP[INET_ADDRSTRLEN];
    int port;

    len = sizeof address;
    getpeername(socket, (struct sockaddr *)&address, &len);

    struct sockaddr_in *s = (struct sockaddr_in *)&address;
    port = ntohs(s->sin_port);
    inet_ntop(AF_INET, &s->sin_addr, IP, sizeof(IP));

    return std::string(std::string(IP) + std::string(" :: ") + std::to_string(s->sin_port));
}

void *testRoutine(void *threadData)
{
    std::vector<SKelegramConnectionPoolData> *connectionsData = (std::vector<SKelegramConnectionPoolData> *)threadData;

    while (1)
    {
    }
}