#include "./headers/SKelegramCore.hpp"

void SKelegramCore::initialize()
{
    connectionPool = new ConnectionPool;
}

std::string SKelegramCore::elaborateRawData(SKelegramRawData skelegramRawData)
{

    ML::log_info(std::string("RAW::INPUT -> ") + skelegramRawData.rawData);

    return skelegramRawData.rawData;
};

void SKelegramCore::handleIncomingConnection(int clientSocket)
{
    connectionPool->addReceiver(clientSocket);
}
