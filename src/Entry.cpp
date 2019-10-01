#include "./Includer.hpp"
#include "./headers/Server.hpp"

int main()
{
    Server *SKelegramServer = new Server();
    
    SKelegramServer->initialize();

    if(SKelegramServer->bindAndListen() < 0){
        ML::log_fatal("Cannot bind or listen with this configuratio... \n Think about checking ports...",TARGET_ALL);
        exit(1);
    };

    SKelegramServer->startAccept();

    return 0;
}
