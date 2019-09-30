#include "./headers/ConnectionThread.hpp"
#define WELCOME_MESSAGE " ################################# \n # Welcome in SKelegram chat :) # \n ################################# \n"

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

    connectionData.incomingMessage.clear();
    connectionData.incomingMessageFlag = 0;

    connectionsData.push_back(connectionData);

    if (pthread_create(&receiverThread, NULL, receiveRoutine, (void *)&connectionsData.back()) == 0)
    {
        threads.push_back(receiverThread);
        
        connectionsCounter++;

        send(connectionData.clientSocket,&WELCOME_MESSAGE,sizeof(WELCOME_MESSAGE),0);
    }
}

void *receiveRoutine(void *threadData)
{
    ML::log_info("Receiver thread set up...", TARGET_ALL);

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

//void *sendRoutine(void *threadData)
//{
//    ML::log_info("Sender thread set up...", TARGET_ALL);
//
//    ConnectionData *connectionData = (ConnectionData *)threadData;
//    connectionData->senderThread = pthread_self();
//
//    while (connectionData->alive)
//    {
//        if (connectionData->toSendMessageFlag && !connectionData->toSendMessage.empty())
//        {
//            send(connectionData->clientSocket, connectionData->toSendMessage.c_str(), strlen(connectionData->toSendMessage.c_str()), 0);
//            ML::log_info(std::string("< ") + ConnectionThreadPool::getConnectionIPAndPort(connectionData->clientSocket) + std::string(" > OUT : ") + connectionData->toSendMessage, TARGET_ALL);
//            connectionData->toSendMessageFlag = 0;
//        }
//    }
//}

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
                    send(connectionsData->at(_index).clientSocket, connectionsData->at(index).incomingMessage.c_str(), strlen(connectionsData->at(index).incomingMessage.c_str()), 0);

                    ML::log_info("Broadcasting" + std::string(" -> ") + ConnectionThreadPool::getConnectionIPAndPort(connectionsData->at(_index).clientSocket), TARGET_ALL);
                }

                connectionsData->at(index).incomingMessageFlag = 0;
            }
        }
    }
}

void *testRoutine(void *threadData)
{
    std::vector<ConnectionData> *connectionsData = (std::vector<ConnectionData> *)threadData;

    while (1)
    {
        //for (std::vector<ConnectionData>::iterator broadcastConnection = connectionsData->begin(); broadcastConnection != connectionsData->end(); broadcastConnection++)
        //{
        //    broadcastConnection->toSendMessage = "PING\0";
        //    broadcastConnection->toSendMessageFlag = 1;
        //}
        //std::this_thread::sleep_for (std::chrono::milliseconds(1000));
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