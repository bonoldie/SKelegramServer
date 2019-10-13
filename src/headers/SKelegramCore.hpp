#ifndef CHATCORE_H
#define CHATCORE_H

#include "../Includer.hpp"

typedef struct sockaddr_in SOCKETADDRIN;

// DEFINE THE RAW DATA ACCEPTED FORMAT
// By index : 0 -> FULL MATCH (TO IGNORE)
//            1 -> TARGET
//            2 -> PAYLOAD
#define RAWDATAFORMATREGEX "(?:\\&\\((.*?)\\)\\&)(.*?)(?:\\&\\(end\\)\\&)"
#define MAXCONNECTIONS 8

// STRUCTURES
// This section contains all the structures that are used in SKelegramServer

// Dictive type
enum SKelegramDataTarget
{
    // Target various server actions
    SERVER,
    // Target to broadcast a message
    BROADCAST,
    // Target chat pools
    CHAT,
    // Invalid target ... will be discard
    INVALID
};

// Low level Data structure
struct SKelegramRawData
{
    // Client socket related to the raw data
    int clientSocket;

    // Raw data contains the incoming data
    // It's encapsulate into a string 
    std::string rawData;
};

// High level Data structure
struct SKelegramInstruction
{
    // Define the target of the raw message
    SKelegramDataTarget target;

    // A pair of instruction and value is created for each raw data structure
    std::string payload;

    int socketFrom;
};

// High level message Data structure
// For future storage implementations
struct SKelegramMessage
{
    std::string username;
    std::string message;
    std::time_t dateTime;
};

// Connection structure to initialize the connection pool
struct SKelegramConnectionPoolData
{
    int *listenSocket;
    SOCKETADDRIN *listenAddress;
    
    std::array<int,MAXCONNECTIONS> * clientSockets;
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
    ConnectionPool(int listenSocket,SOCKETADDRIN listenAddress);
    ~ConnectionPool() = default;

    // Start a new thread for handle a new connection for a give socket.
    // It starts a new receive thread and register socket for Broadcast Routine
    void addReceiver(int clientSocket);

    // Return a string with format "" IP :: PORT "" of a given socket connection
    static std::string getConnectionIPAndPort(int socket);

    void broadcastData(SKelegramRawData data);

    std::vector<SKelegramRawData> rawData;
    std::array<int,MAXCONNECTIONS> registeredSockets;
    int socketsCounter;
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

    // Initialize the core
    void initialize(int serverSocket,SOCKETADDRIN serverAddress);

    // Handle incoming connection from a client 
    void handleIncomingConnection(int clientSocket);

    // Parse raw data into an higher level data structure
    // It's used into the elabora Routine 
    static SKelegramInstruction parseInstruction(SKelegramRawData data);

    // Parse an instruction into raw data structure
    // It's used into the raw data router Routine 
    static SKelegramRawData parseRawData(SKelegramInstruction instruction);
    
    // Contains instruction that need to be elaborated by the core
    std::vector<SKelegramInstruction> toElaborateData;

    // UNUSED FOR NOW
    std::map<int,int> socketChatIDs;

    ConnectionPool *connectionPool;
private:
   
    std::vector<SKelegramMessage> skelegramMessages;
};

#endif