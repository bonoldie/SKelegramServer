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
    pthread_detach(pthread_self()); 

    ConnectionData *threadData = (ConnectionData *)connectionData;

    ML::log_info(std::string("Client connected ") + ConnectionThreadPool::getConnectionIPAndPort(threadData->clientSocket), TARGET_ALL);

    threadData->owner = pthread_self();

    threadData->messageAvailable = false;
    char charBuffer[1];
    bool isReceiving;

    while (1)
    {
        if (!threadData->messageAvailable)
        {
            if ((isReceiving = read(threadData->clientSocket, &charBuffer, 1)) > 0)
            {
                threadData->incomingMessage = std::string(charBuffer);

                while (isReceiving)
                {
                    if ((read(threadData->clientSocket, &charBuffer, 1)) > 0)
                    {
                        threadData->incomingMessage = threadData->incomingMessage + std::string(charBuffer);
                    }
                    isReceiving = threadData->incomingMessage.find("&(end)&") == std::string::npos;
                }

                threadData->incomingMessage += '\0';

                ML::log_info(std::string("< ") + ConnectionThreadPool::getConnectionIPAndPort(threadData->clientSocket) + std::string(" > IN : ") + threadData->incomingMessage, TARGET_ALL);

                threadData->messageAvailable = true;
            }
        }

        while (threadData->toSendBuffer.size() > 0)
        {
            send(threadData->clientSocket, threadData->toSendBuffer.front().c_str(), strlen(threadData->toSendBuffer.front().c_str()), 0);
            ML::log_info(std::string("< ") + ConnectionThreadPool::getConnectionIPAndPort(threadData->clientSocket) + std::string(" > OUT : ") + threadData->toSendBuffer.front(), TARGET_ALL);
            threadData->toSendBuffer.erase(threadData->toSendBuffer.begin());
        }
    }
    pthread_exit(NULL);	
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
