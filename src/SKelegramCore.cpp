#include "./headers/SKelegramCore.hpp"

void SKelegramCore::initialize(int serverSocket,SOCKETADDRIN serverAddress)
{
    connectionPool = new ConnectionPool(serverSocket,serverAddress);
}

void SKelegramCore::handleIncomingConnection(int clientSocket)
{
    connectionPool->addReceiver(clientSocket);
}

SKelegramInstruction SKelegramCore::parseInstruction(SKelegramRawData data)
{
    ML::log_info(data.rawData);
    SKelegramInstruction skInstruction;

    skInstruction.socketFrom = data.clientSocket;

    std::string directiveRegexString("[^\\&\\(]+(?=\\)\\&)");
    std::regex directivesRegex(directiveRegexString);

    // Check if the raw data format is valid

    std::sregex_iterator directiveSearch(data.rawData.begin(), data.rawData.end(), directivesRegex);
    std::sregex_iterator end  = std::sregex_iterator();

    // Check if element exists and directives are exactly 2
    if (directiveSearch == end || std::distance(directiveSearch,end) != 2)
    {
        skInstruction.target = INVALID;
        return skInstruction;
    }

    // Parse the directive or return INVALID directive
    if (directiveSearch->str() == "server")
    {
        skInstruction.target = SERVER;
    }
    else if (directiveSearch->str() == "message")
    {
        skInstruction.target = BROADCAST;
    }else if (directiveSearch->str() == "chat")
    {
        skInstruction.target = CHAT;
    }
    else
    {
        skInstruction.target = INVALID;
        return skInstruction;
    }

    // Check if element exists
    if (directiveSearch++ == end)
    {
        skInstruction.target = INVALID;
        return skInstruction;
    }

    if (directiveSearch++->str() != "end")
    {
        skInstruction.target = INVALID;
        return skInstruction;
    }

    skInstruction.payload = "valid";

    std::string payloadRegexString ("[^\\)\\&]+(?=\\&\\()");
    std::regex payloadRegex (payloadRegexString);

    std::sregex_iterator payloadSeach = std::sregex_iterator(data.rawData.begin(), data.rawData.end(), payloadRegex);

    skInstruction.payload = payloadSeach->str();

    return skInstruction;
}

SKelegramRawData SKelegramCore::parseRawData(SKelegramInstruction instruction)
{

    SKelegramRawData rawData;

    rawData.rawData = std::string("&(message)&") + instruction.payload + std::string("&(end)&");

    return rawData;
}
