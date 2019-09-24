#include "./headers/Server.hpp"

void Server::initialize()
{
    ML::initialize("socketChat.log");

    ML::log_info("Socket Chat loggin system initialized", TARGET_ALL);
}

int Server::bindAndListen(int *socketFD, SOCKETADDRIN *address)
{
    int temp = 1;
    if (*socketFD < 0 || setsockopt(*socketFD, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &temp, sizeof(temp)) < 0)
    {
        ML::log_fatal(std::string("Cannot create new socket with current IP or port !"), TARGET_ALL);
        return -1;
    }

    ML::log_info("Server socket created", TARGET_ALL);

    // Building address (SOCKETADDRIN) for listeninig
    address->sin_addr.s_addr = INADDR_ANY;
    address->sin_port = htons(port);
    address->sin_family = AF_INET;

    if (bind(*socketFD, (struct sockaddr *)address, sizeof(*address)) < 0 || listen(*socketFD, 8) < 0)
    {
        ML::log_fatal("Binding or Listening failed", TARGET_ALL);
        return -1;
    }

    ML::log_info(std::string("Listening for connections at port ") + std::to_string(port), TARGET_ALL);

    return 1;
}

// Start a new thread to handle a connection with a client  
// @param socket the client socket 
void *Server::handleConnection(void *socket)
{
    char buffer[10];
    int clientSocket = *((int *)socket);

    ML::log_info(std::string("Client connected ") + getConnectionIPAndPort(clientSocket), TARGET_ALL);

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
            addMessageToThreads(receivedMessage);
        }

        while (threadData[clientSocket].size() > 0)
        {
            send(clientSocket, threadData[clientSocket].front().c_str(), strlen(threadData[clientSocket].front().c_str()), 0);
            threadData[clientSocket].erase(threadData[clientSocket].begin());
        }
    }
}


std::string Server::getConnectionIPAndPort(int socket)
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