#ifndef CHATCORE_H
#define CHATCORE_H

#include "../Includer.hpp"

#define MAXCONNECTIONS 8

// STRUCTURES
// This section contains all the structures that are used in SKelegramServer

// Dictive type
enum Directive
{
    SERVER,
    BROADCAST
};

// Low level Data structure
struct SKelegramRawData
{
       int clientSocket;
    std::string rawData;
};

// Medium level Data structure
struct SKelegramData
{
    int elaborated = 0;
    unsigned int connectionPoolID;
    SKelegramRawData data;
};

// High level message Data structure
struct SKelegramMessage
{
    std::string username;
    std::string message;
    std::time_t dateTime;
};

// Connection structure to initialize new connection
struct SKelegramConnectionPoolData
{
    std::vector<int> * clientSockets;
    std::vector<SKelegramRawData> *rawData;
    int *isReady;
};

// Data passed to connection thread pool Broadcaster
struct BroadcastData
{
    int *isThreadReady;
    std::vector<SKelegramRawData> *broadcastRawData;
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
    static uint poolCounter;
    const uint poolID;

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
    void registerConnectionPool(ConnectionPool *connectionPool);
    void handleIncomingConnection(int clientSocket);

    std::vector<ConnectionPool *> connectionPools;
    
    std::vector<SKelegramData> queuedData;
private:
    std::vector<SKelegramMessage> skelegramMessages;
};

#endif