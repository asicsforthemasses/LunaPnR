// SPDX-FileCopyrightText: 2021-2024 Niels Moseley <asicsforthemasses@gmail.com>
//
// SPDX-License-Identifier: GPL-3.0-only

#include <iostream>
#include <sstream>
#include <cstdio>
#include <cstdarg>
#include <array>
#include <string_view>
#include "logging.h"

namespace Logging
{


class DefaultLogOutputHandler : public Logging::LogOutputHandler
{
public:
    void print(LogType level, const std::string &txt) override
    {
        print(level, std::string_view(txt));
    }

    void print(LogType level, const std::string_view &txt) override
    {
        std::stringstream ss;
        switch(level)
        {
        case LogType::PRINT:
            break;
        case LogType::INFO:
            ss << FGINFO   << "[INFO] ";
            break;
        case LogType::DEBUG:
            ss << FGDEBUG  << "[DBG ] ";
            break;
        case LogType::WARNING:
            ss << FGWARN   << "[WARN] ";
            break;
        case LogType::ERROR:
            ss << FGERR    << "[ERR ] ";
            break;
        case LogType::VERBOSE:
            ss << FGVERB   << "[VERB] ";
            break;
        default:
            break;
        }
        ss << txt;
        std::cout << ss.str() << FGDEFAULT;
    }

protected:

    static constexpr const char* FGRED     = "\033[38;5;9m";
    static constexpr const char* FGGREEN   = "\033[38;5;2m";
    static constexpr const char* FGYELLOW  = "\033[38;5;11m";
    static constexpr const char* FGNAVY    = "\033[38;5;6m";
    static constexpr const char* FGWHITE   = "\033[38;5;15m";
    static constexpr const char* FGDEFAULT = "\033[m";

    static constexpr const char* FGDEBUG = FGNAVY;
    static constexpr const char* FGERR   = FGRED;
    static constexpr const char* FGWARN  = FGYELLOW;
    static constexpr const char* FGINFO  = FGGREEN;
    static constexpr const char* FGVERB  = FGWHITE;

};

static DefaultLogOutputHandler gs_defaultLogHandler;
static LogType gs_loglevel = LogType::WARNING;
static LogOutputHandler *gs_logOutputHandler = nullptr;

void setOutputHandler(Logging::LogOutputHandler *handler)
{
    gs_logOutputHandler = handler;
}

void setLogLevel(LogType level)
{
    gs_loglevel = level;
}

LogType getLogLevel()
{
    return gs_loglevel;
}

void doLog(LogType t, const char *format, va_list args)
{
    if (t < gs_loglevel)
    {
        return;
    }

    std::array<char, 2048> buffer;
    buffer.at(0) = 0;

    vsnprintf(&buffer[0], buffer.size(), format, args);

    if (gs_logOutputHandler == nullptr)
    {
        gs_defaultLogHandler.print(t, std::string_view(&buffer[0]));
    }
    else
    {
        gs_logOutputHandler->print(t, std::string_view(&buffer[0]));
    }
}

void logError(std::string_view fmt, ...)
{
    std::va_list args;
    va_start(args, fmt);
    doLog(Logging::LogType::ERROR, fmt.data(), args);
    va_end(args);
}

void logWarning(std::string_view fmt, ...)
{
    std::va_list args;
    va_start(args, fmt);
    doLog(Logging::LogType::WARNING, fmt.data(), args);
    va_end(args);
}

void logVerbose(std::string_view fmt, ...)
{
    std::va_list args;
    va_start(args, fmt);
    doLog(Logging::LogType::VERBOSE, fmt.data(), args);
    va_end(args);
}

void logDebug(std::string_view fmt, ...)
{
    std::va_list args;
    va_start(args, fmt);
    doLog(Logging::LogType::DEBUG, fmt.data(), args);
    va_end(args);
}

void logPrint(std::string_view fmt, ...)
{
    std::va_list args;
    va_start(args, fmt);
    doLog(Logging::LogType::PRINT, fmt.data(), args);
    va_end(args);
}

void logInfo(std::string_view fmt, ...)
{
    std::va_list args;
    va_start(args, fmt);
    doLog(Logging::LogType::INFO, fmt.data(), args);
    va_end(args);
}

}; //namespace
