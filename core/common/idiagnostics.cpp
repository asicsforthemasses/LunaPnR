// SPDX-FileCopyrightText: 2021-2025 Niels Moseley <asicsforthemasses@gmail.com>
//
// SPDX-License-Identifier: GPL-3.0-only

#include "idiagnostics.hpp"

namespace LunaCore
{

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

};