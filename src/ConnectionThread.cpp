#include "./headers/ConnectionThread.hpp"

ConnectionThreadPool::ConnectionThreadPool()
{
}

void ConnectionThreadPool::addConnectionThread(int clientSocket)
{
    ConnectionData connectionData;
    connectionData.clientSocket = clientSocket;
    connectionsData.push_back(connectionData);

    pthread_create(&threads[index], NULL, handleConnection, (void *)&connectionsData.back());

    index++;
    threadCounter++;
}

void *handleConnection(void *connectionData)
{

    ConnectionData *threadData = (ConnectionData *)connectionData;

    ML::log_info(std::string("Client connected ") + ConnectionThreadPool::getConnectionIPAndPort(threadData->clientSocket), TARGET_ALL);

    char buffer[10];
    std::string receivedMessage;

    while (1)
    {
        receivedMessage.clear();

        bool isReceiving = recv(threadData->clientSocket, &buffer, 1, 0);

        while (isReceiving)
        {
            while (int size = recv(threadData->clientSocket, &buffer, 1, 0) > 0)
            {
                if (size == 1)
                {
                    receivedMessage += buffer[0];
                }
                isReceiving = receivedMessage.find("&(end)&") == std::string::npos;
            }
        }
        if (!receivedMessage.empty())
        {
            threadData->receivedBuffer.push_back(receivedMessage);
            ML::log_info(std::string("Rec ... ") + std::string(threadData->receivedBuffer.back()), TARGET_ALL);
        }

        while (threadData->toSendBuffer.size() > 0)
        {
            send(threadData->clientSocket, threadData->toSendBuffer.front().c_str(), strlen(threadData->toSendBuffer.front().c_str()), 0);
            ML::log_info(std::string("Sending ... ") + std::string(threadData->toSendBuffer.front()), TARGET_ALL);
            threadData->toSendBuffer.erase(threadData->toSendBuffer.begin());
        }
    }
}

void ConnectionThreadPool::clearRecData(){
    for(int i = 0;i < connectionsData.size();i++ ){
        connectionsData[i].receivedBuffer.clear();
    }
}

std::string ConnectionThreadPool::getConnectionIPAndPort(int socket)
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
