#include "./headers/SKelegramServer.hpp"

void SKelegramServer::initialize()
{
    ML::initialize("socketChat.log");

    ML::log_info("Socket Chat loggin system initialized");

    skelegramCore = new SKelegramCore();

    skelegramCore->initialize();

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

    std::vector<SKelegramRawData> *toElaborateData = &instance->toElaborateData;

    while (1)
    {

        // Take raw data from connection pool raw data  buffer and add it to toElaborate vector

        while (toElaborateData->size() > 0)
        {
            if (!toElaborateData->front().elaborated)
            {
                if (toElaborateData->front().rawData == "&(server)&CLOSECONNECTION&(end)&" || toElaborateData->front().rawData.find("&(end)&") == std::string::npos)
                {
                    toElaborateData->erase(toElaborateData->begin());
                }
                else
                {
                    toElaborateData->front().elaborated = 1;
                }
            }
        }
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(400));
}

void *rawDataRouterRoutine(void *coreInstance)
{
    SKelegramCore *instance = (SKelegramCore *)coreInstance;

    std::vector<SKelegramRawData> *toElaborateData = &instance->toElaborateData;

    ML::log_info("Raw data Router initialized on CORE");

    while (1)
    {
        while (instance->connectionPool->rawData.size() > 0)
        {
            toElaborateData->push_back(instance->connectionPool->rawData.front());
            instance->connectionPool->rawData.erase(instance->connectionPool->rawData.begin());
        }

        while (toElaborateData->size() > 0)
        {
            if (toElaborateData->front().elaborated)
            {
                instance->connectionPool->broadcastData(instance->toElaborateData.front().rawData);
                toElaborateData->erase(toElaborateData->begin());
            }
        }
    }
}
