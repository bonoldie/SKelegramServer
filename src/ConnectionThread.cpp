#include "./headers/ConnectionThread.hpp"

ConnectionThreadPool::ConnectionThreadPool()
{
    pthread_create(&broadcastThread, NULL, broadcastRoutine, (void *)&connectionsData);
}

void ConnectionThreadPool::addConnectionThread(int clientSocket)
{
    ML::log_info(std::string("Client connection from ") + ConnectionThreadPool::getConnectionIPAndPort(clientSocket), TARGET_ALL);

    pthread_t receiverThread, senderThread;
    ConnectionData connectionData;

    connectionData.clientSocket = clientSocket;
    connectionData.alive = 1;

    connectionData.incomingMessage = connectionData.toSendMessage = "";
    connectionData.incomingMessageFlag = connectionData.toSendMessageFlag = 0;

    connectionsData.push_back(connectionData);

    ML::log_info(std::string("Pre threading ... "), TARGET_ALL);

    pthread_create(&receiverThread, NULL, receiveRoutine, (void *)&connectionsData.back());
    pthread_create(&senderThread, NULL, sendRoutine, (void *)&connectionsData.back());

    ML::log_info(std::string("Post threading ... "), TARGET_ALL);

    connectionsCounter++;
}

void *receiveRoutine(void *threadData)
{
    ML::log_info("Receiver thread set up...", TARGET_ALL);
    pthread_detach(pthread_self());

    ConnectionData *connectionData = (ConnectionData *)threadData;
    connectionData->receiverThread = pthread_self();

    std::string tempString;

    char buffer[1];

    while (connectionData->alive)
    {
        if (connectionData->incomingMessageFlag == 0)
        {
            tempString.clear();

            while (tempString.find("&(end)&") == std::string::npos)
            {
                if (recv(connectionData->clientSocket, &buffer, 1, 0) > 0)
                {
                    tempString += buffer[0];
                }
            }

            ML::log_info(std::string("< ") + ConnectionThreadPool::getConnectionIPAndPort(connectionData->clientSocket) + std::string(" > IN : ") + tempString, TARGET_ALL);

            connectionData->incomingMessage = tempString + '\0';
            connectionData->incomingMessageFlag = 1;
        }
    }
}

void *sendRoutine(void *threadData)
{
    ML::log_info("Sender thread set up...", TARGET_ALL);
    pthread_detach(pthread_self());

    ConnectionData *connectionData = (ConnectionData *)threadData;
    connectionData->senderThread = pthread_self();

    while (connectionData->alive)
    {
        if (connectionData->toSendMessageFlag && !connectionData->toSendMessage.empty())
        {
                send(connectionData->clientSocket, connectionData->toSendMessage.c_str(), strlen(connectionData->toSendMessage.c_str()), 0);
                ML::log_info(std::string("< ") + ConnectionThreadPool::getConnectionIPAndPort(connectionData->clientSocket) + std::string(" > OUT : ") + connectionData->toSendMessage, TARGET_ALL);
                connectionData->toSendMessage.clear();
        }
    }
}

void *broadcastRoutine(void *threadData)
{
    std::vector<ConnectionData> *connectionsData = (std::vector<ConnectionData> *)threadData;

    while (1)
    {
        for (int index = 0; index < connectionsData->size(); index++)
        {
            if (connectionsData->at(index).incomingMessageFlag)
            {

                for (int _index = 0; _index < connectionsData->size(); _index++)
                {
                    connectionsData->at(_index).toSendMessage = connectionsData->at(index).incomingMessage;
                    connectionsData->at(_index).toSendMessageFlag = 1;

                    ML::log_info("Broadcasting" + std::string(" -> ") + ConnectionThreadPool::getConnectionIPAndPort(connectionsData->at(index).clientSocket), TARGET_ALL);
                }
                connectionsData->at(index).incomingMessageFlag = 0;
            }
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

//void *handleConnection(void *connectionData)
//{
//    pthread_detach(pthread_self());
//
//    ConnectionData *threadData = (ConnectionData *)connectionData;
//
//    ML::log_info(std::string("Client connected ") + ConnectionThreadPool::getConnectionIPAndPort(threadData->clientSocket), TARGET_ALL);
//
//    threadData->owner = pthread_self();
//
//    threadData->messageAvailable = false;
//    char charBuffer[1];
//    bool isReceiving;
//
//    while (1)
//    {
//        if (!threadData->messageAvailable)
//        {
//            if ((isReceiving = recv(threadData->clientSocket, &charBuffer, 1, 0)) > 0)
//            {
//                threadData->incomingMessage = std::string(charBuffer);
//
//                while (isReceiving)
//                {
//                    if ((read(threadData->clientSocket, &charBuffer, 1)) > 0)
//                    {
//                        threadData->incomingMessage = threadData->incomingMessage + std::string(charBuffer);
//                    }
//                    isReceiving = threadData->incomingMessage.find("&(end)&") == std::string::npos;
//                }
//
//                threadData->incomingMessage += '\0';
//
//                ML::log_info(std::string("< ") + ConnectionThreadPool::getConnectionIPAndPort(threadData->clientSocket) + std::string(" > IN : ") + threadData->incomingMessage, TARGET_ALL);
//
//                threadData->messageAvailable = true;
//            }
//        }
//
//        while (threadData->toSendMessages.size() > 0)
//        {
//            send(threadData->clientSocket, threadData->toSendMessages.front().c_str(), strlen(threadData->toSendMessages.front().c_str()), 0);
//            ML::log_info(std::string("< ") + ConnectionThreadPool::getConnectionIPAndPort(threadData->clientSocket) + std::string(" > OUT : ") + threadData->toSendMessages.front(), TARGET_ALL);
//            threadData->toSendMessages.erase(threadData->toSendMessages.begin());
//        }
//    }
//    pthread_exit(NULL);
//}