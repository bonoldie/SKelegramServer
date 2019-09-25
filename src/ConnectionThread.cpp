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

    std::string incomingMessageStream = "";

    threadData->messageAvailable = false;
    char charBuffer[1];

    bool isReceiving;

    while (1)
    {
        if (!threadData->messageAvailable)
        {
            if ((isReceiving = read(threadData->clientSocket, &charBuffer, 1)) > 0)
            {

                incomingMessageStream = std::string(charBuffer);

                while (isReceiving)
                {
                    if ((read(threadData->clientSocket, &charBuffer, 1)) > 0)
                    {
                        incomingMessageStream = incomingMessageStream + std::string(charBuffer);
                    }
                    isReceiving = incomingMessageStream.find("&(end)&") == std::string::npos;
                }

                incomingMessageStream += '\0';
                ML::log_info(std::string("Recived in temp : ") + incomingMessageStream, TARGET_ALL);

                threadData->toSendBuffer.push_back(incomingMessageStream);

                ML::log_info(std::string("Recived in data : ") + threadData->incomingMessages[0], TARGET_ALL);
                threadData->messageAvailable = true;
            }
        }

        while (threadData->toSendBuffer.size() > 0)
        {
            send(threadData->clientSocket, threadData->toSendBuffer.front().c_str(), strlen(threadData->toSendBuffer.front().c_str()), 0);
            ML::log_info(std::string("Sending : ") + threadData->toSendBuffer.front(), TARGET_ALL);
            threadData->toSendBuffer.erase(threadData->toSendBuffer.begin());
        }
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
