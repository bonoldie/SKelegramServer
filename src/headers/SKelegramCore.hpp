#ifndef CHATCORE_H
#define CHATCORE_H

#include "../Includer.hpp"

enum Directive{
    SERVER,
    BROADCAST
};

struct SKelegramMessage {
    std::string username;
    std::string message;
    std::time_t dateTime; 
};

class SKelegramCore {
public:
    SKelegramCore() = default;
    ~SKelegramCore() = default;
    
    std::string elaborateRawData(std::string rawData);
private:
    std::vector<SKelegramMessage> skelegramMessages;
};


#endif