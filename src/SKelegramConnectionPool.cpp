#include "./headers/SKelegramCore.hpp"
#define WELCOME_MESSAGE " ################################# \n # Welcome in SKelegram chat :) # \n ################################# \n"

std::mutex commonMutex;

ConnectionPool::ConnectionPool()
{
    pthread_t poolThread;
    SKelegramConnectionPoolData poolData;

    poolData.clientSockets = &registeredSockets;
    poolData.rawData = &rawData;
    *poolData.isReady = 0;

    pthread_create(&poolThread, NULL, poolReceiveRoutine, (void *)&poolData);

    while (*poolData.isReady == 0)
        ;
}

void ConnectionPool::addReceiver(int clientSocket)
{
    ML::log_info(std::string("Client connected : ") + ConnectionPool::getConnectionIPAndPort(clientSocket));

    // Using mutex for operate with shared data
    commonMutex.lock();

    registeredSockets.push_back(clientSocket);

    commonMutex.unlock();

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
        commonMutex.lock();
        std::vector<int> sockets = *connectionData.clientSockets;
        commonMutex.unlock();

        for (int currentSocket : sockets)
        {
            receivedString.clear();

            char buffer[1];

            if ((receivedFlag = recv(currentSocket, &buffer, 1, 0)) == 0)
            {
                // Sending close connection message to upper layer
                receivedString = "&(server)&CLOSECONNECTION&(end)&";

                // Deleting the socket
                auto delSockets = std::find(connectionData.clientSockets->begin(), connectionData.clientSockets->end(), currentSocket);

                if (delSockets != connectionData.clientSockets->end())
                {
                    connectionData.clientSockets->erase(delSockets);
                }

                ML::log_info(std::string("Client disconnected : ") + ConnectionPool::getConnectionIPAndPort(currentSocket));
            }
            else if (receivedFlag > 0)
            {
                receivedString += buffer[0];

                while (receivedString.find("&(end)&") == std::string::npos && (receivedFlag = recv(currentSocket, &buffer, 1, 0)) > 0)
                {
                    receivedString += buffer[0];
                }
            }

            // if receivedFlag is 0 -> connection is close
            // otherwise if receivedString contains end delimiter it's added to the raw data queue
            if (receivedString.find("&(end)&") != std::string::npos)
            {
                // Setup new raw data
                SKelegramRawData receivedRawData;
                receivedRawData.rawData = receivedString;
                receivedRawData.clientSocket = currentSocket;

                // Adding to pool common raw data buffer
                commonMutex.lock();
                connectionData.rawData->push_back(receivedRawData);
                commonMutex.unlock();
                ML::log_info(std::string("Added raw data -> ") + receivedRawData.rawData);
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