#ifndef SKELEGRAMLOGGIN_H
#define SKELEGRAMLOGGIN_H

#include "../Includer.hpp"
#include "./SKelegramCore.hpp"

namespace SKL{
    void log_info(int socket,std::string message);
    void log_warning(int socket,std::string message);
    void log_error(int socket,std::string message);
    void log_fatal(int socket,std::string message);
}

#endif