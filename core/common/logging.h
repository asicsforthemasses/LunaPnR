// SPDX-FileCopyrightText: 2021-2025 Niels Moseley <asicsforthemasses@gmail.com>
//
// SPDX-License-Identifier: GPL-3.0-only

#pragma once
#include <cstdint>
#include <string>
#include <sstream>
#include <string_view>

#ifdef __GNUC__
#include <signal.h>
#define BREAK_HERE raise(SIGINT)
#endif

namespace Logging
{

enum class LogType : uint8_t
{
    VERBOSE = 1,
    DEBUG = 2,
    INFO = 3,
    WARNING = 4,
    ERROR = 8,
    PRINT = 100
};

/** subclass LogOutputHandler to provide your own output processing
 *  you must provide multi-threading support yourself.
*/
struct LogOutputHandler
{
    virtual void print(LogType level, const std::string &txt) = 0;
    virtual void print(LogType level, const std::string_view &txt) = 0;
};

/** set the log level ... */
void setLogLevel(LogType level);

/** get the log level ... */
LogType getLogLevel();

/** set new log output handler */
void setOutputHandler(LogOutputHandler *handler);

void logError(std::string_view fmt, ...);
void logWarning(std::string_view fmt, ...);
void logVerbose(std::string_view fmt, ...);
void logDebug(std::string_view fmt, ...);
void logPrint(std::string_view fmt, ...);
void logInfo(std::string_view fmt, ...);

};
