#ifndef MLOGGER_H
#define MLOGGER_H

// Imports
#include <iostream>
#include <fstream>
#include <memory>
#include <chrono>
#include <sstream>
#include <string.h>
#include <iomanip>

#define COUT_SLASH << "-" <<
#define COUT_COLONS << ":" <<

#define COUT_DOUBLE_DATE std::setfill('0') << std::setw(2) <<

#define TARGET_FILE 1
#define TARGET_CONSOLE 2
#define TARGET_ALL 3

/**
 * Mlogger is a lightweight Loggin system
 * */
namespace ML
{
class MLogger;

enum LogLevel
{
    INFO,
    WARNING,
    ERROR,
    FATAL
};

class MLogger
{

public:
    MLogger() = default;

    void initialize(char *pathToLogFile = 0);

    void logToFile(const char *log);

    void log(const char *textToLog, LogLevel logLevel, int target);

    inline static MLogger *getMLogger()
    {
        static std::unique_ptr<MLogger> instance = std::make_unique<MLogger>();
        return instance.get();
    }

    void setLogPath(char *_pathToLogFile);

    void showDate();

    void hideDate();

private:
    static std::unique_ptr<MLogger> instance;

    std::ofstream logFileStream;

    char *logFilePath = "";

    bool showDateAndTime = true;

    char *logLevelS[5] = {
        " /// TO CORRECT BUG ENTRY ///",
        " INFO ",
        " WARNING ",
        " ERROR ",
        " FATAL "};
};

extern void initialize(char *pathToLogFile = 0);
extern void setLogPath(char *pathToLogFile = 0);

extern void showDate();
extern void hideDate();

extern void log_info(const char *textToLog, int target = TARGET_ALL);
extern void log_warning(const char *textToLog, int target = TARGET_ALL);
extern void log_error(const char *textToLog, int target = TARGET_ALL);
extern void log_fatal(const char *textToLog, int target = TARGET_ALL);

extern void log_info(std::string textToLog, int target = TARGET_ALL);
extern void log_warning(std::string textToLog, int target = TARGET_ALL);
extern void log_error(std::string textToLog, int target = TARGET_ALL);
extern void log_fatal(std::string textToLog, int target = TARGET_ALL);

} // namespace ML

#endif