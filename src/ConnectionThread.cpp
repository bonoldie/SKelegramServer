#include "./headers/ConnectionThread.hpp"

void ConnectionThreadPool::addConnectionThread(int clientSocket)
{
    clientSockets[thID] = clientSocket;
    pthread_create(&connectionThreads[thID], NULL, (THREADFUNCPTR)&ConnectionThreadPool::handleConnection, &thID);

    thID++;
}

// Start a new thread to handle a connection with a client  
// @param socket the client socket 
void *ConnectionThreadPool::handleConnection(void *socket)
{
    char buffer[10];
    int clientSocket = *((int *)socket);

    ML::log_info(std::string("Client connected ") + ConnectionThreadPool::getConnectionIPAndPort(clientSocket), TARGET_ALL);

    std::string receivedMessage;

    while (1)
    {
        receivedMessage.clear();

        bool isReceiving = recv(clientSocket, &buffer, 1, 0);

        if(isReceiving){
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
            //addMessageToThreads(receivedMessage);
        }

        while (threadData[clientSocket].size() > 0)
        {
            send(clientSocket, threadData[clientSocket].front().c_str(), strlen(threadData[clientSocket].front().c_str()), 0);
            threadData[clientSocket].erase(threadData[clientSocket].begin());
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