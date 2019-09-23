#include "./Includer.hpp"

#define MAXCONNECTIONS 255
#define PORT 45678

typedef struct sockaddr_in SOCKETADDRIN;

void initialize();
int bindAndListen(int *socketFD, SOCKETADDRIN *address);
void *handleConnection(void *socket);
void addMessageToThreads(std::string message);

std::map<int,std::vector<std::string>> threadData = {};

int main()
{
    initialize();

    int threadCounter = 0;
    std::array<pthread_t, MAXCONNECTIONS> threads;

    int serverSocketFD;
    SOCKETADDRIN listeningAddress;
    int clientSockets[MAXCONNECTIONS];
    int addressSize = sizeof(listeningAddress);

    serverSocketFD = socket(AF_INET, SOCK_STREAM, 0);

    if (bindAndListen(&serverSocketFD, &listeningAddress) < 0)
    {
        ML::log_error("Error while binding or listening to the socket...exiting", TARGET_ALL);
    }

    while (1)
    {
        clientSockets[threadCounter] = accept(serverSocketFD, (struct sockaddr *)&listeningAddress, (socklen_t *)&addressSize);

        if (clientSockets[threadCounter] > 0)
        {
            if (pthread_create(&threads[threadCounter], NULL, handleConnection, &clientSockets[threadCounter]) == 0)
            {
                threadData[clientSockets[threadCounter]] = {};
                threadCounter++;
            }
            else
            {
                ML::log_error("Error while accepting connection", TARGET_ALL);
            }
        }
    }

    return 0;
}

void initialize()
{
    ML::initialize("socketChat.log");

    ML::log_info("Socket Chat loggin system initialized", TARGET_ALL);
}

// Bind and start listening to a specific SOCKET with a certain address
// Return 1 on success , -1 otherwise
int bindAndListen(int *socketFD, SOCKETADDRIN *address)
{
    int temp = 1;
    if (*socketFD < 0 || setsockopt(*socketFD, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &temp, sizeof(temp)) < 0)
    {
        ML::log_fatal(std::string("Cannot create new socket with current IP or port !"), TARGET_ALL);
        return -1;
    }

    ML::log_info("Server socket created", TARGET_ALL);

    // Building address (SOCKETADDRIN) for listeninig
    address->sin_addr.s_addr = INADDR_ANY;
    address->sin_port = htons(PORT);
    address->sin_family = AF_INET;

    if (bind(*socketFD, (struct sockaddr *)address, sizeof(*address)) < 0 || listen(*socketFD, 8) < 0)
    {
        ML::log_fatal("Binding or Listening failed", TARGET_ALL);
        return -1;
    }

    ML::log_info(std::string("Listening for connections at port ") + std::to_string(PORT), TARGET_ALL);

    return 1;
}

void *handleConnection(void *socket)
{
    ML::log_info("Client connected ",TARGET_ALL);

    char buffer[1000];
    int clientSocket = *((int *)socket);

    while (1)
    {
        while (read(clientSocket, &buffer, 1000) > 0)
        {
            addMessageToThreads(std::string(buffer));
        }

        while (threadData[clientSocket].size() > 0)
        {
            send(clientSocket, threadData[clientSocket].front().c_str(), strlen(threadData[clientSocket].front().c_str()), 0);
            threadData[clientSocket].erase(threadData[clientSocket].begin());
        }
    }
}


void addMessageToThreads(std::string message){
    for(std::map<int,std::vector<std::string>>::iterator it = threadData.begin();it != threadData.end();it++){
        it->second.push_back(message);
    }
}