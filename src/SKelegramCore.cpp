#include "./headers/SKelegramCore.hpp"

void SKelegramCore::initialize()
{
    pthread_t elaborateRoutine;
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
