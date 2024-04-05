// SPDX-FileCopyrightText: 2021-2024 Niels Moseley <asicsforthemasses@gmail.com>
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

/** Log type, used to filter log messages */
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

/** Set the log level ...
    The default level is LogType::INFO
*/
void setLogLevel(LogType level);

/** get the log level ... */
LogType getLogLevel();

/** set new log output handler */
void setOutputHandler(LogOutputHandler *handler);

/** log an error with printf format string */
void logError(std::string_view fmt, ...);

/** log a warning with printf format string */
void logWarning(std::string_view fmt, ...);

/** log a verbose message with printf format string */
void logVerbose(std::string_view fmt, ...);

/** log a debug message with printf format string */
void logDebug(std::string_view fmt, ...);

/** log a message with printf format string */
void logPrint(std::string_view fmt, ...);

/** log a informative message with printf format string */
void logInfo(std::string_view fmt, ...);

};
