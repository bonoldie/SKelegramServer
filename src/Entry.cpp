#include "./Includer.hpp"
#include "./headers/SKelegramServer.hpp"

int main()
{
    SKelegramServer *skelegramServer = new SKelegramServer();
    
    skelegramServer->initialize();

    if(skelegramServer->bindAndListen() < 0){
        ML::log_fatal("Cannot bind or listen with this configuratio... \n Think about checking ports..." );
        exit(1);
    };

    skelegramServer->startAccept();

    while(1){

    }
    
    return 0;
}
