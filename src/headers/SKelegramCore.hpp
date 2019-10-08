#ifndef CHATCORE_H
#define CHATCORE_H

#include "../Includer.hpp"

#define MAXCONNECTIONS 8

// STRUCTURES
// This section contains all the structures that are used in SKelegramServer

// Dictive type
enum SKelegramDataTarget
{
    SERVER,
    BROADCAST
};

struct SKelegramInstruction
{
    SKelegramDataTarget target;
    std::string instruction;
    std::string value; 
};

// Low level Data structure
struct SKelegramRawData
{
    // When created raw data is not elaborated 
    int elaborated = 0;

    // Client socket related to the raw data
    int clientSocket;

    // Chat ID define the broadcast range
    int chatID;

    // Raw data contains the incoming data
    // It's encapsulate into a string 
    std::string rawData;
};

// High level message Data structure
struct SKelegramMessage
{
    std::string username;
    std::string message;
    std::time_t dateTime;
};

// Connection structure to initialize the connection pool
struct SKelegramConnectionPoolData
{
    std::vector<int> * clientSockets;
    std::vector<SKelegramRawData> *rawData;
    int *isReady;
};

// SKelegramCore DEFINITIONS

// Receive Routine
// It check for incoming message from a given soket
// Then add it to common incoming messages array
void *poolReceiveRoutine(void *threadData);

// Useful test routine for connection pools
void *testRoutine(void *threadData);

// ConnectionPool define a pool of connections
// I/O with low level data
class ConnectionPool
{
public:
    ConnectionPool();
    ~ConnectionPool() = default;

    // Start a new thread for handle a new connection for a give socket.
    // It starts a new receive thread and register socket for Broadcast Routine
    void addReceiver(int clientSocket);

    // Return a string with format "" IP :: PORT "" of a given socket connection
    static std::string getConnectionIPAndPort(int socket);

    void broadcastData(std::string rawData);

    std::vector<SKelegramRawData> rawData;
    std::vector<int> registeredSockets;

private:
    pthread_t broadcastThread;
};

// Core routine
void *elaborateDataRoutine(void *incomingData);

// Collect data from all registered pools and add that to elaborating queue
void *rawDataRouterRoutine(void *coreInstance);

// SKelegramCore define the core of the application
// It handles connections and elaborate data
// Any IO action
class SKelegramCore
{
public:
    SKelegramCore() = default;
    ~SKelegramCore() = default;

    void initialize();

    std::string elaborateRawData(SKelegramRawData rawData);
    void handleIncomingConnection(int clientSocket);

    ConnectionPool *connectionPool;
    std::vector<SKelegramRawData> toElaborateData;

    std::map<int,int> socketChatIDs;
private:
   
    std::vector<SKelegramMessage> skelegramMessages;
};

#endif