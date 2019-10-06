#include "./headers/SKelegramCore.hpp"

void SKelegramCore::initialize()
{
    pthread_t elaborateRoutine;
    pthread_create(&elaborateRoutine, NULL, &elaborateDataRoutine, (void *)&queuedData);
}

void SKelegramCore::registerConnectionPool(ConnectionPool *connectionPool)
{
    connectionPools.push_back(connectionPool);
}

std::string SKelegramCore::elaborateRawData(SKelegramRawData skelegramRawData)
{

    ML::log_info(std::string("RAW::INPUT -> ") + skelegramRawData.rawData);

    return skelegramRawData.rawData;
};

void SKelegramCore::handleIncomingConnection(int clientSocket)
{
    if (connectionPools.size() > 0)
    {
        connectionPools.front()->addReceiver(clientSocket);
    }
    else
    {
        ML::log_warning("Cannot handle incoming connection request. No pools available!");
    }
}

void *elaborateDataRoutine(void *incomingData)
{
    std::vector<SKelegramData> *toElaborateData = (std::vector<SKelegramData> *)incomingData;
    while (1)
    {
        if (toElaborateData->size() > 0)
        {
            if (!toElaborateData->front().elaborated)
            {
                // FOR NOW IT ONLY BROADCAST INCOMING MESSAGES

                toElaborateData->front().elaborated = 1;
            }
        }
    }
};