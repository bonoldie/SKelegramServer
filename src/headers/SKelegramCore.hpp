#ifndef CHATCORE_H
#define CHATCORE_H

#include "../Includer.hpp"
#include "./ConnectionThread.hpp"

// STRUCTURES
// This section contains all the structures that are used in SKelegramServer

enum Directive{
    SERVER,
    BROADCAST
};

struct SKelegramRawData{
    unsigned int connectionID;
    std::string rawData;
};

struct SKelegramData{      
    unsigned int connectionPoolID;
    SKelegramRawData data;
};

struct SKelegramMessage {
    std::string username;
    std::string message;
    std::time_t dateTime; 
};

// SKelegramCore DEFINITIONS

class SKelegramCore {
public:
    SKelegramCore() = default;
    ~SKelegramCore() = default;
    
    std::string elaborateRawData(std::string rawData);
private:
    std::vector<SKelegramMessage> skelegramMessages;
};



#endif