#include "./headers/SKelegramCore.hpp"

void SKelegramCore::initialize()
{
    pthread_t elaborateRoutine;
    pthread_create(&elaborateRoutine,NULL,&elaborateDataRoutine,(void*)&incomingData);
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
        connectionPools.at(0)->addReceiver(clientSocket);
    }
    else
    {
        ML::log_warning("Cannot handle incoming connection request. No pools available!");
    }
}

void *elaborateDataRoutine(void *incomingData){
    std::vector<SKelegramData> * toElaborateData = (std::vector<SKelegramData> *)incomingData;
    while(1){
        if(toElaborateData->size() > 0){
            // FOR NOW IT ONLY BROADCAST INCOMING MESSAGES
            send(toElaborateData->at(0).data.clientSocket,toElaborateData->at(0).data.rawData.c_str(),strlen(toElaborateData->at(0).data.rawData.c_str()),0);
            // Removing last element 
           toElaborateData->erase(toElaborateData->begin());
        }
    }
};