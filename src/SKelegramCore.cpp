#include "./headers/SKelegramCore.hpp"

void SKelegramCore::initialize(int serverSocket, SOCKETADDRIN serverAddress)
{
    connectionPool = new ConnectionPool(serverSocket, serverAddress);
}

void SKelegramCore::handleIncomingConnection(int clientSocket)
{
    connectionPool->addReceiver(clientSocket);
}

SKelegramInstruction SKelegramCore::parseInstruction(SKelegramRawData data)
{
    SKelegramInstruction skInstruction;

    skInstruction.socketFrom = data.clientSocket;

    // Check if the the raw data is formatted correctly
    std::string rawDataFormat(RAWDATAFORMATREGEX);
    std::regex rawDataFormatRegex(rawDataFormat);

    std::smatch rawMatches;

    if (std::regex_search(data.rawData, rawMatches, rawDataFormatRegex))
    {
        // True if raw data is formatted correctly
        if (rawMatches.size() == 3)
        {

            // Parsing TARGET
            if (rawMatches[1].str() == "server")
            {
                skInstruction.target = SERVER;
            }
            else if (rawMatches[1].str() == "message")
            {
                skInstruction.target = BROADCAST;
            }
            else if (rawMatches[1].str() == "chat")
            {
                skInstruction.target = CHAT;
            }
            else
            {
                skInstruction.target = INVALID;
                skInstruction.payload = "INVALIDTARGET --> ";
                skInstruction.payload += rawMatches[1].str();
                return skInstruction;
            }

            skInstruction.payload = rawMatches[2];
        }
        else
        {
            skInstruction.target = INVALID;
            skInstruction.payload = "MALFORMEDDATA --> ";
            skInstruction.payload = data.rawData;
            return skInstruction;
        }
    }
    else
    {

        skInstruction.target = INVALID;
        skInstruction.payload = "MALFORMEDDATA --> ";
        skInstruction.payload = data.rawData;
        return skInstruction;
    }

    return skInstruction;
}

SKelegramRawData SKelegramCore::parseRawData(SKelegramInstruction instruction)
{

    SKelegramRawData rawData;

    rawData.rawData = std::string("&(message)&") + instruction.payload + std::string("&(end)&");

    return rawData;
}
