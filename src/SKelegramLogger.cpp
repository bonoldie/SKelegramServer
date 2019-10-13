#include "./headers/SKelegramLogger.hpp"

void SKL::log_info(int socket, std::string message)
{
    ML::log_info(std::string("< ") + ConnectionPool::getConnectionIPAndPort(socket) + std::string(" > ") + message);
}
void SKL::log_warning(int socket, std::string message)
{
    ML::log_warning(std::string("< ") + ConnectionPool::getConnectionIPAndPort(socket) + std::string(" > ") + message);
}
void SKL::log_error(int socket, std::string message)
{
    ML::log_error(std::string("< ") + ConnectionPool::getConnectionIPAndPort(socket) + std::string(" > ") + message);
}
void SKL::log_fatal(int socket, std::string message)
{
    ML::log_fatal(std::string("< ") + ConnectionPool::getConnectionIPAndPort(socket) + std::string(" > ") + message);
}