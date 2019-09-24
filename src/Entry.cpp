#include "./Includer.hpp"
#include "./headers/Server.hpp"

int main()
{
    Server *SKelegramServer = new Server();
    
    SKelegramServer->initialize();
    SKelegramServer->bindAndListen();
    SKelegramServer->startAccept();

    return 0;
}
