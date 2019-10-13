#include "./headers/SKelegramServer.hpp"

void SKelegramServer::initialize()
{
    ML::initialize("socketChat.log");

    ML::log_info("Socket Chat loggin system initialized");

    skelegramCore = new SKelegramCore();

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

    listenAddress.sin_addr.s_addr = INADDR_ANY;
    listenAddress.sin_port = htons(port);
    listenAddress.sin_family = AF_INET;

    if (bind(serverSocketFD, (struct sockaddr *)&listenAddress, sizeof(listenAddress)) < 0 || listen(serverSocketFD, 8) < 0)
    {
        ML::log_fatal("Binding or Listening failed");
        return -1;
    }

    ML::log_info(std::string("Listening for connections at port ") + std::to_string(port));

    return 0;
}

void SKelegramServer::startAccept()
{
    skelegramCore->initialize(serverSocketFD, listenAddress);

    pthread_t elaboratorThread;

    pthread_create(&elaboratorThread, NULL, &elaborateDataRoutine, (void *)skelegramCore);
}

void *elaborateDataRoutine(void *coreInstance)
{
    SKelegramCore *instance = (SKelegramCore *)coreInstance;
    std::vector<SKelegramInstruction> *toElaborateData = &instance->toElaborateData;
    while (1)
    {
        // Take raw data from connection pool raw data  buffer and add it to toElaborate vector
        while (instance->connectionPool->rawData.size() > 0)
        {
            toElaborateData->push_back(SKelegramCore::parseInstruction(instance->connectionPool->rawData.front()));
            instance->connectionPool->rawData.erase(instance->connectionPool->rawData.begin());
        }
        // Check for not elaborated data to process
        while (toElaborateData->size() > 0)
        {
            switch (toElaborateData->front().target)
            {
            case SERVER:
                SKL::log_info(toElaborateData->front().socketFrom, toElaborateData->front().payload);

                if (toElaborateData->front().payload == "CONNECTIONCLOSED")
                {
                    for (int socketReg = 0; socketReg < instance->connectionPool->registeredSockets.size(); socketReg++)
                    {
                        if (instance->connectionPool->registeredSockets[socketReg] == toElaborateData->front().socketFrom)
                        {
                            instance->connectionPool->registeredSockets[socketReg] = instance->connectionPool->registeredSockets[instance->connectionPool->socketsCounter--];
                        }
                    }
                }
                if (toElaborateData->front().payload == "CLIENTCONNECTED")
                {
                    instance->connectionPool->registeredSockets[instance->connectionPool->socketsCounter] = toElaborateData->front().socketFrom;
                    instance->connectionPool->socketsCounter++;
                }

                break;
            case CHAT:
                break;
            case BROADCAST:
                instance->connectionPool->broadcastData(SKelegramCore::parseRawData(toElaborateData->front()));
                break;
            case INVALID:
                SKL::log_error(toElaborateData->front().socketFrom, toElaborateData->front().payload);
                break;
            }
            toElaborateData->erase(toElaborateData->begin());
        }
    }
}