// SPDX-FileCopyrightText: 2021-2025 Niels Moseley <asicsforthemasses@gmail.com>
//
// SPDX-License-Identifier: GPL-3.0-only

#pragma once
#include <sstream>
#include "strutils.hpp"

namespace LunaCore
{

/** base class for sending human readable information to the user */
class IDiagnostics
{
public:
    virtual ~IDiagnostics() = default;

    virtual void info(const char *fmt, ...)  =  0;
    virtual void debug(const char *fmt, ...) =  0;
    virtual void error(const char *fmt, ...) =  0;
    virtual void warn(const char *fmt, ...)  =  0;

    void info(u32sstream &txt);
    void debug(u32sstream &txt);
    void error(u32sstream &txt);
    void warn(u32sstream &txt);

    void info(std::stringstream &txt);
    void debug(std::stringstream &txt);
    void error(std::stringstream &txt);
    void warn(std::stringstream &txt);

    void info(std::string &txt);
    void debug(std::string &txt);
    void error(std::string &txt);
    void warn(std::string &txt);

    void info(std::u32string &txt);
    void debug(std::u32string &txt);
    void error(std::u32string &txt);
    void warn(std::u32string &txt);

};

};