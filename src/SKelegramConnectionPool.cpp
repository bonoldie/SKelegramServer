#include "./headers/SKelegramCore.hpp"
#define WELCOME_MESSAGE " ################################# \n # Welcome in SKelegram chat :) # \n ################################# \n"

std::mutex commonMutex;

ConnectionPool::ConnectionPool(int listenSocket, SOCKETADDRIN listenAddress)
{
    pthread_t poolThread;
    SKelegramConnectionPoolData poolData;

    poolData.clientSockets = &registeredSockets;
    poolData.rawData = &rawData;

    poolData.isReady = (int *)malloc(sizeof(int));
    poolData.listenSocket = (int *)malloc(sizeof(int));
    poolData.listenAddress = (SOCKETADDRIN *)malloc(sizeof(SOCKETADDRIN));

    *poolData.isReady = 0;
    *poolData.listenSocket = listenSocket;
    *poolData.listenAddress = listenAddress;

    pthread_create(&poolThread, NULL, poolReceiveRoutine, (void *)&poolData);

    while (*poolData.isReady == 0);
}

void ConnectionPool::addReceiver(int clientSocket)
{

}

void *poolReceiveRoutine(void *threadData)
{
    int fdsCounter, listenSocket, pollValue;
    SOCKETADDRIN listenAddress;
    size_t listenAddressSize;

    // Cast to correct type
    SKelegramConnectionPoolData connectionData = *(SKelegramConnectionPoolData *)threadData;

    // Retrive server socket to handle
    listenSocket = *connectionData.listenSocket;
    listenAddress = *connectionData.listenAddress;
    listenAddressSize = sizeof(listenAddress);

    *connectionData.isReady = 1;

    fcntl(listenSocket, F_SETFL, O_NONBLOCK);

    std::map<int, std::string> rawPartialData;

    // Set up fds for polling
    pollfd fds[MAXCONNECTIONS];
    pollfd listenPollFD;

    listenPollFD.fd = listenSocket;
    listenPollFD.events = POLLIN;

    fds[0] = listenPollFD;

    fdsCounter = 1;

    while (1)
    {
        pollValue = poll(fds, fdsCounter, -1);

        // Poll has changed state
        if (pollValue > 0)
        {
            for (int fdIndex = 0; fdIndex < fdsCounter; fdIndex++)
            {
                // If the event is setted on the server socket it means a new connection is coming
                if (fds[fdIndex].fd == listenSocket && fds[fdIndex].revents == POLLIN)
                {
                    int newClientSocket = accept(listenSocket, (struct sockaddr *)&listenAddress, (socklen_t *)&listenAddressSize);

                    pollfd newConnectionFD;

                    newConnectionFD.fd = newClientSocket;
                    newConnectionFD.events = POLLIN;

                    fds[fdsCounter] = newConnectionFD;

                    rawPartialData[newClientSocket] = "&(server)&CLIENTCONNECTED&(end)&";

                    ML::log_info(std::string("Client connection from ") + ConnectionPool::getConnectionIPAndPort(newClientSocket));



                    fdsCounter++;
                }
                // Else it checks connections FD and find who rise the poll
                else
                {
                    // There is data to read
                    if (fds[fdIndex].revents == POLLIN)
                    {
                        int readState;
                        char buffer[1];

                        if ((readState = read(fds[fdIndex].fd, &buffer, 1)) > 0)
                        {
                            rawPartialData[fds[fdIndex].fd] += buffer;

                            ML::log_info(std::string("received from client  : ") + rawPartialData[fds[fdIndex].fd]);
                        }
                        // If readState is 0 it means socket is closed so we clean out the socket
                        else if (readState == 0)
                        {
                            ML::log_info(std::string("Connection closed : ") + ConnectionPool::getConnectionIPAndPort(fds[fdIndex].fd));

                            rawPartialData[fds[fdIndex].fd] = "&(server)&CONNECTIONCLOSED&(end)&";

                            rawPartialData.erase(fds[fdIndex].fd);
                            close(fds[fdIndex].fd);
                            fds[fdIndex] = fds[fdsCounter--];

                            pollfd nullPoll;
                            fds[fdsCounter] = nullPoll;

                            
                        }
                    }
                }
            }
        }

        for (std::map<int, std::string>::iterator rawDataIterator = rawPartialData.begin(); rawDataIterator != rawPartialData.end(); rawDataIterator++)
        {
            if (rawDataIterator->second.find("&(end)&") != std::string::npos)
            {
                SKelegramRawData rawData;
                rawData.rawData = rawDataIterator->second;
                rawData.clientSocket = rawDataIterator->first;

                commonMutex.lock();
                connectionData.rawData->push_back(rawData);
                commonMutex.unlock();

                rawDataIterator->second = "";
            }
        }
    }
}

void ConnectionPool::broadcastData(SKelegramRawData data)
{
    for (int clientSocket : registeredSockets)
    {
        send(clientSocket, data.rawData.c_str(), strlen(data.rawData.c_str()), 0);
    }
}

std::string ConnectionPool::getConnectionIPAndPort(int socket)
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

void *testRoutine(void *threadData)
{
    std::vector<SKelegramConnectionPoolData> *connectionsData = (std::vector<SKelegramConnectionPoolData> *)threadData;

    while (1)
    {
    }
}