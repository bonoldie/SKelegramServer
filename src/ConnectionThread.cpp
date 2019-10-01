#include "./headers/ConnectionThread.hpp"
#define WELCOME_MESSAGE " ################################# \n # Welcome in SKelegram chat :) # \n ################################# \n"

ConnectionThreadPool::ConnectionThreadPool()
{
    BroadcastData bcData;
    bcData.sockets = &sockets;
    bcData.incomingMessagges = &incomingMessagges;
    pthread_create(&broadcastThread, NULL, broadcastRoutine, (void *)&bcData);
}

// Start a new thread for handle a new connection for a give socket.
// It starts a new receive thread and regester socket for Broadcast Routine
void ConnectionThreadPool::addConnectionThread(int clientSocket)
{
    ML::log_info(std::string("Client connection from ") + ConnectionThreadPool::getConnectionIPAndPort(clientSocket), TARGET_ALL);

    pthread_t receiverThread, senderThread;
    ConnectionData connectionData;

    connectionData.clientSocket = clientSocket;
    connectionData.incomingMessages = &incomingMessagges;

    sockets.push_back(clientSocket);

    if (pthread_create(&receiverThread, NULL, receiveRoutine, (void *)&connectionData) == 0)
    {
        threads.push_back(receiverThread);

        connectionsCounter++;

        send(connectionData.clientSocket, &WELCOME_MESSAGE, sizeof(WELCOME_MESSAGE), 0);
    }
}

// Receive Routine
// It check for incoming message from a given soket
// Then add it to common incoming messages array
void *receiveRoutine(void *threadData)
{
    ML::log_info("Receiver thread set up...", TARGET_ALL);

    ConnectionData connectionData = *(ConnectionData *)threadData;

    std::string tempString;

    char buffer[1];

    while (1)
    {
        tempString.clear();

        while (tempString.find("&(end)&") == std::string::npos)
        {
            if (recv(connectionData.clientSocket, &buffer, 1, 0) > 0)
            {
                tempString += buffer[0];
            }
        }

        connectionData.incomingMessages->push_back(tempString);

        ML::log_info(std::string("< ") + ConnectionThreadPool::getConnectionIPAndPort(connectionData.clientSocket) + std::string(" > IN : ") + connectionData.incomingMessages->back(), TARGET_ALL);
    }
}

// Broadcast Routine
// Checks for incoming messages and send them to all registered sockets
// In works as echo server for now.
void *broadcastRoutine(void *threadData)
{
    BroadcastData broadcastData = *(BroadcastData *)threadData;

    while (1)
    {
        if (broadcastData.incomingMessagges->size() > 0)
        {
            for (int _index = 0; _index < broadcastData.sockets->size(); _index++)
            {
                send(broadcastData.sockets->at(_index), broadcastData.incomingMessagges->front().c_str(), strlen(broadcastData.incomingMessagges->front().c_str()), 0);

                ML::log_info("Broadcasting" + std::string(" -> ") + ConnectionThreadPool::getConnectionIPAndPort(broadcastData.sockets->at(_index)), TARGET_ALL);
            }
            broadcastData.incomingMessagges->erase(broadcastData.incomingMessagges->begin());
        }
    }
}

// Return a string with format "" IP :: PORT "" of a given socket connection
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

// TESTING PURPOSE ROUTINE //
void *testRoutine(void *threadData)
{
    std::vector<ConnectionData> *connectionsData = (std::vector<ConnectionData> *)threadData;

    while (1)
    {
        
    }
}