#include "./headers/ConnectionThread.hpp"

ConnectionThreadPool::ConnectionThreadPool()
{
    for (int i = 0; i < MAXCONNECTIONS; i++)
    {
        IDs[i] = i + 1;
    }
}

void ConnectionThreadPool::addConnectionThread(int clientSocket)
{
    clientSockets[thID] = clientSocket;

    int *arg = (int*)malloc(sizeof(*arg));
    
    pthread_create(&connectionThreads[thID], NULL, (THREADFUNCPTR)&ConnectionThreadPool::handleConnection, (void*)&thID);
    threadMessages[thID] = {};
    thID = IDs[thID];
}

// Start a new thread to handle a connection with a client
// @param socket the client socket
void *ConnectionThreadPool::handleConnection(void *thIDt)
{
    char buffer[10];
    if((int *)thIDt == nullptr){
        exit(1);
    }
    int parsedThID = *((int *)thIDt);
    int clientSocket = clientSockets[parsedThID];
 
    ML::log_info(std::string("Client connected ") + ConnectionThreadPool::getConnectionIPAndPort(clientSocket), TARGET_ALL);
    std::string receivedMessage;
    
    while (1)
    {
        receivedMessage.clear();

        bool isReceiving = recv(clientSocket, &buffer, 1, 0);

        if (isReceiving)
        {
            receivedMessage += buffer[0];
        }

        while (isReceiving)
        {
            while (int size = recv(clientSocket, &buffer, 1, 0) > 0)
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
            ML::log_info(receivedMessage, TARGET_ALL);
            addMessageToThreads(receivedMessage);
        }

        while (threadMessages[parsedThID].size() > 0)
        {
            send(clientSocket, threadMessages[parsedThID].front().c_str(), strlen(threadMessages[parsedThID].front().c_str()), 0);
            threadMessages[parsedThID].erase(threadMessages[parsedThID].begin());
        }
    }
}

void ConnectionThreadPool::addMessageToThreads(std::string message)
{
    for (int i = 0; i < threadMessages.size(); i++)
    {
        threadMessages[i].push_back(message);
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