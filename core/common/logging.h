/*
    LunaPnR Source Code
  
    SPDX-License-Identifier: GPL-3.0-only
    SPDX-FileCopyrightText: 2022 Niels Moseley <asicsforthemasses@gmail.com>
*/

#pragma once
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

/** log something using standard C printf format varargs - limit 2049 chars */
void doLog(LogType t, const char *format, ...);

/** log something using standard C++ std::string - limit 2049 chars */
void doLog(LogType t, const std::string &txt);

/** log something using standard C++ std::stringstream - no limit */
void doLog(LogType t, const std::stringstream &txt);

std::string fmt(const char *format, ...);

/** set the log level ... */
void setLogLevel(LogType level);

/** get the log level ... */
LogType getLogLevel();

/** set new log output handler */
void setOutputHandler(LogOutputHandler *handler);

};
