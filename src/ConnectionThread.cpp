#include "./headers/ConnectionThread.hpp"
#define WELCOME_MESSAGE " ################################# \n # Welcome in SKelegram chat :) # \n ################################# \n"

ConnectionThreadPool::ConnectionThreadPool()
{
    BroadcastData bcData;
    bcData.sockets = &sockets;
    *bcData.isThreadReady = 0;
    // Connect directly broadcast messages to incoming messages 
    //  |
    //  V
    //bcData.broadcastMessages = &incomingMessages;
    // Separate incoming and to broacast messages for future implementations
    //  |
    //  V
    bcData.broadcastMessages = &broadcastMessages;
    pthread_create(&broadcastThread, NULL, broadcastRoutine, (void *)&bcData);

    // Waiting for thread to clean scope
    // (bad way)
    while(*bcData.isThreadReady == 0){ ML::log_warning("! Waiting for Broadcast thread startup !",TARGET_ALL);};
}

// Start a new thread for handle a new connection for a give socket.
// It starts a new receive thread and register socket for Broadcast Routine
void ConnectionThreadPool::addConnectionThread(int clientSocket)
{
    ML::log_info(std::string("Client connection from ") + ConnectionThreadPool::getConnectionIPAndPort(clientSocket), TARGET_ALL);

    pthread_t receiverThread, senderThread;
    SKelegramConnetion connectionData;

    connectionData.clientSocket = clientSocket;
    connectionData.rawData = &rawData;

    sockets.push_back(clientSocket);

    if (pthread_create(&receiverThread, NULL, receiveRoutine, (void *)&connectionData) == 0)
    {
        threads.push_back(receiverThread);

        send(connectionData.clientSocket, &WELCOME_MESSAGE, sizeof(WELCOME_MESSAGE), 0);
    }
}

// Receive Routine
// It check for incoming message from a given soket
// Then add it to common incoming messages array
void *receiveRoutine(void *threadData)
{
    SKelegramConnetion connectionData = *(SKelegramConnetion *)threadData;

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

        connectionData.rawData->push_back(tempString);
    }
}

// Broadcast Routine
// Checks for broacast messages and send them to all registered sockets
void *broadcastRoutine(void *threadData)
{
    BroadcastData broadcastData = *(BroadcastData *)threadData;

    *broadcastData.isThreadReady = 1;

    while (1)
    {
        if (broadcastData.broadcastMessages->size() > 0)
        {
            for (int _index = 0; _index < broadcastData.sockets->size(); _index++)
            {
                send(broadcastData.sockets->at(_index), broadcastData.broadcastMessages->front().c_str(), strlen(broadcastData.broadcastMessages->front().c_str()), 0);
            }
            broadcastData.broadcastMessages->erase(broadcastData.broadcastMessages->begin());
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
    std::vector<SKelegramConnetion> *connectionsData = (std::vector<SKelegramConnetion> *)threadData;

    while (1)
    {
        
    }
}