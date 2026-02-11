// SPDX-FileCopyrightText: 2021-2025 Niels Moseley <asicsforthemasses@gmail.com>
//
// SPDX-License-Identifier: GPL-3.0-only

#include "idiagnostics.hpp"
#include <cstdio>
#include <cstdarg>

namespace LunaCore
{

#if 0    
void IDiagnostics::info(u32sstream &txt)
{
    info("%s", toUTF8(txt.str()).c_str());
}

void IDiagnostics::debug(u32sstream &txt)
{
    debug("%s", toUTF8(txt.str()).c_str());
}

void IDiagnostics::error(u32sstream &txt)
{
    error("%s", toUTF8(txt.str()).c_str());
}

void IDiagnostics::warn(u32sstream &txt)
{
    warn("%s", toUTF8(txt.str()).c_str());
}

void IDiagnostics::info(std::stringstream &txt)
{
    info("%s", txt.str().c_str());
}

void IDiagnostics::debug(std::stringstream &txt)
{
    debug("%s", txt.str().c_str());
}

void IDiagnostics::error(std::stringstream &txt)
{
    error("%s", txt.str().c_str());
}

void IDiagnostics::warn(std::stringstream &txt)
{
    warn("%s", txt.str().c_str());
}

void IDiagnostics::info(std::string &txt)
{
    info("%s", txt.c_str());
}

void IDiagnostics::debug(std::string &txt)
{
    debug("%s", txt.c_str());
}

void IDiagnostics::error(std::string &txt)
{
    error("%s", txt.c_str());
}

void IDiagnostics::warn(std::string &txt)
{
    warn("%s", txt.c_str());
}

void IDiagnostics::info(std::u32string &txt)
{
    info("%s", toUTF8(txt).c_str());
}

void IDiagnostics::debug(std::u32string &txt)
{
    debug("%s", toUTF8(txt).c_str());
}

void IDiagnostics::error(std::u32string &txt)
{
    error("%s", toUTF8(txt).c_str());
}

void IDiagnostics::warn(std::u32string &txt)
{
    warn("%s", toUTF8(txt).c_str());
}
#endif

void ConsoleDiagnostics::info(const char *fmt, ...)
{
    if (m_enableANSIColors)
        printf("%s[INFO] %s", ANSI::white, ANSI::normal);
    else
        printf("[INFO] ");

    va_list lst;
    va_start(lst, fmt);
    vprintf(fmt, lst);
    va_end(lst);
}

void ConsoleDiagnostics::debug(const char *fmt, ...)
{
    if (m_enableANSIColors)
        printf("%s[DBG ] %s", ANSI::green, ANSI::normal);
    else 
        printf("[DBG ] ");

    va_list lst;
    va_start(lst, fmt);
    vprintf(fmt, lst);
    va_end(lst);
}

void ConsoleDiagnostics::error(const char *fmt, ...)
{
    if (m_enableANSIColors)
        printf("%s[ERR ] %s", ANSI::red, ANSI::normal);
    else
        printf("[ERR ] ");

    va_list lst;
    va_start(lst, fmt);
    vprintf(fmt, lst);
    va_end(lst);
}

void ConsoleDiagnostics::warn(const char *fmt, ...)
{
    if (m_enableANSIColors)
        printf("%s[WARN] %s", ANSI::yellow, ANSI::normal);
    else
        printf("[WARN] ");

    va_list lst;
    va_start(lst, fmt);
    vprintf(fmt, lst);
    va_end(lst);
}

};