#include "./headers/SKelegramCore.hpp"
#define WELCOME_MESSAGE " ################################# \n # Welcome in SKelegram chat :) # \n ################################# \n"

uint ConnectionPool::poolCounter = 0;

ConnectionPool::ConnectionPool() : poolID(poolCounter++)
{
}

void ConnectionPool::addReceiver(int clientSocket)
{
    ML::log_info(std::string("Client connected from ") + ConnectionPool::getConnectionIPAndPort(clientSocket));

    pthread_t receiverThread;
    SKelegramConnetion connectionData;

    connectionData.clientSocket = clientSocket;
    connectionData.rawData = &rawData;

    if (pthread_create(&receiverThread, NULL, receiveRoutine, (void *)&connectionData) == 0)
    {
        registeredSockets.push_back(clientSocket);

        send(connectionData.clientSocket, &WELCOME_MESSAGE, sizeof(WELCOME_MESSAGE), 0);
    }
}

void *receiveRoutine(void *threadData)
{
    SKelegramConnetion connectionData = *(SKelegramConnetion *)threadData;

    std::string receivedString;

    char buffer[1];

    int receivedFlag,isRunning = 1;

    while (isRunning)
    {
        receivedString.clear();

        while (receivedString.find("&(end)&") == std::string::npos && isRunning)
        {
            if ((receivedFlag = recv(connectionData.clientSocket, &buffer, 1, 0)) > 0)
            {
                receivedString += buffer[0];
            }
            else if(receivedFlag == 0)
            {
                isRunning = 0;
                receivedString = "&(server)&CLOSECONNECTION&(end)&";
            }
        }

        SKelegramRawData receivedRawData;

        receivedRawData.rawData = receivedString;
        receivedRawData.clientSocket = connectionData.clientSocket;

        connectionData.rawData->push_back(receivedRawData);
    }

    ML::log_info(std::string("Connection closed for ") + ConnectionPool::getConnectionIPAndPort(connectionData.clientSocket));
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
    std::vector<SKelegramConnetion> *connectionsData = (std::vector<SKelegramConnetion> *)threadData;

    while (1)
    {
    }
}