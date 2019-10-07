#include "./headers/SKelegramServer.hpp"

void SKelegramServer::initialize()
{
    ML::initialize("socketChat.log");

    ML::log_info("Socket Chat loggin system initialized");

    skelegramCore = new SKelegramCore();

    skelegramCore->initialize();

    connectionPool = new ConnectionPool();

    skelegramCore->registerConnectionPool(connectionPool);

    pthread_t collectorThread, elaboratorThread;

    pthread_create(&collectorThread, NULL, &rawDataRouterRoutine, (void *)skelegramCore);
    pthread_create(&elaboratorThread, NULL, &elaborateDataRoutine, (void *)skelegramCore);

    serverSocketFD = socket(AF_INET, SOCK_STREAM, 0);
}

int SKelegramServer::bindAndListen()
{
    int temp = 1;
    if (serverSocketFD < 0 || setsockopt(serverSocketFD, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &temp, sizeof(temp)) < 0)
    {
        ML::log_fatal(std::string("Cannot set current options on socket !"));
        return -1;
    }

    ML::log_info("Server socket created");

    listeningAddress.sin_addr.s_addr = INADDR_ANY;
    listeningAddress.sin_port = htons(port);
    listeningAddress.sin_family = AF_INET;

    if (bind(serverSocketFD, (struct sockaddr *)&listeningAddress, sizeof(listeningAddress)) < 0 || listen(serverSocketFD, 8) < 0)
    {
        ML::log_fatal("Binding or Listening failed");
        return -1;
    }

    ML::log_info(std::string("Listening for connections at port ") + std::to_string(port));

    return 0;
}

void SKelegramServer::startAccept()
{
    ServerRoutineData serverData;
    serverData.connectionPool = connectionPool;
    serverData.skelegramCore = skelegramCore;

    //pthread_create(&serverRoutineThread, NULL, serverRoutine, (void *)&serverData);

    int addressSize = sizeof(listeningAddress);
    while (1)
    {

        int clientSocket = accept(serverSocketFD, (struct sockaddr *)&listeningAddress, (socklen_t *)&addressSize);
        fcntl(clientSocket, F_SETFL, O_NONBLOCK);

        if (clientSocket > 0)
        {
            skelegramCore->handleIncomingConnection(clientSocket);
        }
    }
}

void *elaborateDataRoutine(void *coreInstance)
{
    SKelegramCore *instance = (SKelegramCore *)coreInstance;

    std::vector<SKelegramData> *toElaborateData = &instance->queuedData;

    while (1)
    {
        if (toElaborateData->size() > 0)
        {
            if (!toElaborateData->front().elaborated)
            {
                if (toElaborateData->front().data.rawData == "&(server)&CLOSECONNECTION&(end)&")
                {
                    for (ConnectionPool *connectionPool : instance->connectionPools)
                    {
                        if (connectionPool->poolID == toElaborateData->front().connectionPoolID)
                        {
                            auto regSockets = std::find(connectionPool->registeredSockets.begin(), connectionPool->registeredSockets.end(), toElaborateData->front().data.clientSocket);

                            if (regSockets != connectionPool->registeredSockets.end())
                            {
                                connectionPool->registeredSockets.erase(regSockets);
                            }
                        }
                    }
                    toElaborateData->erase(toElaborateData->begin());
                }
                else
                {
                    toElaborateData->front().elaborated = 1;
                }

                // FOR NOW IT ONLY BROADCAST INCOMING MESSAGES
            }
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(400));
    }
};

void *rawDataRouterRoutine(void *coreInstance)
{
    SKelegramCore *instance = (SKelegramCore *)coreInstance;

    ML::log_info("Raw data Router initialized on CORE");

    while (1)
    {
        for (ConnectionPool *connectionPool : instance->connectionPools)
        {
            if (connectionPool->rawData.size() > 0)
            {
                //ML::log_info(std::string("Data incoming from") + std::to_string(connectionPool->poolID) + std::string(" : ") + connectionPool->rawData.front().rawData);
                SKelegramData skRawData;
                skRawData.connectionPoolID = connectionPool->poolID;
                skRawData.data = connectionPool->rawData.front();

                instance->queuedData.push_back(skRawData);

                connectionPool->rawData.erase(connectionPool->rawData.begin());
            }
        }

        if (instance->queuedData.size() > 0)
        {
            if (instance->queuedData.front().elaborated)
            {
                for (ConnectionPool *connectionPool : instance->connectionPools)
                {
                    if (connectionPool->poolID == instance->queuedData.front().connectionPoolID)
                    {
                        //ML::log_info(std::string("Data incoming from") + std::to_string(connectionPool->poolID) + std::string(" : ") + connectionPool->rawData.front().rawData);
                        connectionPool->broadcastData(instance->queuedData.front().data.rawData);

                        instance->queuedData.erase(instance->queuedData.begin());
                    }
                }
            }
        }
    }
}
