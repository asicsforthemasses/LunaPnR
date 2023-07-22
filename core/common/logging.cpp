// SPDX-FileCopyrightText: 2021-2022,2023 Niels Moseley <asicsforthemasses@gmail.com>
//
// SPDX-License-Identifier: GPL-3.0-only

#include <iostream>
#include <sstream>
#include <cstdio>
#include <cstdarg>
#include <array>
#include "logging.h"

class DefaultLogOutputHandler : public Logging::LogOutputHandler
{
public:
    void print(Logging::LogType level, const std::string &txt) override
    {
        print(level, std::string_view(txt));
    }

    void print(Logging::LogType level, const std::string_view &txt) override
    {
        std::stringstream ss;
        switch(level)
        {
        case Logging::LogType::PRINT:
            break;
        case Logging::LogType::INFO:
            ss << FGINFO   << "[INFO] ";
            break;
        case Logging::LogType::DEBUG:
            ss << FGDEBUG  << "[DBG ] ";
            break;
        case Logging::LogType::WARNING: 
            ss << FGWARN   << "[WARN] ";
            break;
        case Logging::LogType::ERROR:
            ss << FGERR    << "[ERR ] ";
            break;
        case Logging::LogType::VERBOSE:
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
static Logging::LogType gs_loglevel = Logging::LogType::WARNING;
static Logging::LogOutputHandler *gs_logOutputHandler = nullptr;

void Logging::setOutputHandler(Logging::LogOutputHandler *handler)
{
    gs_logOutputHandler = handler;
}

void Logging::setLogLevel(LogType level)
{
    gs_loglevel = level;
}

Logging::LogType Logging::getLogLevel()
{
    return gs_loglevel;
}

void Logging::doLog(LogType t, const std::string &txt)
{
    Logging::doLog(t, txt.c_str());
}

void Logging::doLog(LogType t, const char *format, ...)
{   
    if (t < gs_loglevel)
    {
        return;
    }

    std::array<char, 2048> buffer;
    buffer.at(0) = 0;

    va_list argptr;
    va_start(argptr, format);
    vsnprintf(&buffer[0], buffer.size(), format, argptr);
    va_end(argptr);
    
    if (gs_logOutputHandler == nullptr)
    {
        gs_defaultLogHandler.print(t, std::string_view(&buffer[0]));
    }
    else
    {
        gs_logOutputHandler->print(t, std::string_view(&buffer[0]));
    }
}

void Logging::doLog(LogType t, const std::stringstream &txt)
{
    if (t < gs_loglevel)
    {
        return;
    }

    if (gs_logOutputHandler == nullptr)
    {
        gs_defaultLogHandler.print(t, txt.str());
    }
    else
    {
        gs_logOutputHandler->print(t, txt.str());
    }
}

std::string Logging::fmt(const char *format, ...)
{
    std::array<char, 2048> buffer;
    buffer.at(0) = 0;

    va_list argptr;
    va_start(argptr, format);
    vsnprintf(&buffer[0], buffer.size(), format, argptr);
    va_end(argptr);
   
    return &buffer[0];
}
