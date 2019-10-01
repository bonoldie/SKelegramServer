#include "./headers/Server.hpp"

// Initialize the server logger and the main thread pool
// Future updates will implement more pools
void Server::initialize()
{
    ML::initialize("socketChat.log");

    ML::log_info("Socket Chat loggin system initialized", TARGET_ALL);

    skelegramCore = new SKelegramCore();

    threadPool = new ConnectionThreadPool();

    serverSocketFD = socket(AF_INET, SOCK_STREAM, 0);
}

// Try to bind and listen to the selected port or if not set !! to the default one DEFAULT_PORT
int Server::bindAndListen()
{
    int temp = 1;
    if (serverSocketFD < 0 || setsockopt(serverSocketFD, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &temp, sizeof(temp)) < 0)
    {
        ML::log_fatal(std::string("Cannot set current options on socket !"), TARGET_ALL);
        return -1;
    }

    ML::log_info("Server socket created", TARGET_ALL);

    listeningAddress.sin_addr.s_addr = INADDR_ANY;
    listeningAddress.sin_port = htons(port);
    listeningAddress.sin_family = AF_INET;

    if (bind(serverSocketFD, (struct sockaddr *)&listeningAddress, sizeof(listeningAddress)) < 0 || listen(serverSocketFD, 8) < 0)
    {
        ML::log_fatal("Binding or Listening failed", TARGET_ALL);
        return -1;
    }

    ML::log_info(std::string("Listening for connections at port ") + std::to_string(port), TARGET_ALL);

    return 0;
}

// Start to accept request of connetion from the binded socket
// It's blocking until any request is rised
void Server::startAccept()
{
    ServerRoutineData serverData;
    serverData.cThreadPool = threadPool;
    serverData.skelegramCore = skelegramCore;

    pthread_create(&serverRoutineThread, NULL, serverRoutine, (void *)&serverData);

    int addressSize = sizeof(listeningAddress);
    while (1)
    {

        int clientSocket = accept(serverSocketFD, (struct sockaddr *)&listeningAddress, (socklen_t *)&addressSize);
        fcntl(clientSocket, F_SETFL, O_NONBLOCK);

        if (clientSocket > 0)
        {
            threadPool->addConnectionThread(clientSocket);
        }
    }
}

void *serverRoutine(void *data)
{
    ServerRoutineData routineData = *(ServerRoutineData *)data;
    ConnectionThreadPool *cThreadPool = routineData.cThreadPool;
    SKelegramCore *skelegramCore = routineData.skelegramCore;

    ML::log_info("Server Routine initialized",TARGET_ALL);

    while (1)
    {
        if (cThreadPool->rawData.size() > 0)
        {
            std::string elaboratedMessage = skelegramCore->elaborateRawData(cThreadPool->rawData.front());
            
            cThreadPool->broadcastMessages.push_back(elaboratedMessage);

            cThreadPool->rawData.erase(cThreadPool->rawData.begin());
        }
    }
}
