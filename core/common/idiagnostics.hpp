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

    virtual void info(const char *fmt, ...)  =  0;  ///< send info to the user, assumes UTF-8.
    virtual void debug(const char *fmt, ...) =  0;  ///< send debug info to the user, assumes UTF-8.
    virtual void error(const char *fmt, ...) =  0;  ///< send error info to the user, assumes UTF-8.
    virtual void warn(const char *fmt, ...)  =  0;  ///< send warning info to the user, assumes UTF-8.


};

/** Diagnostics class that sends everything to an ANSI console. */
class ConsoleDiagnostics : public IDiagnostics
{
public:
    void info(const char *fmt, ...) override;  ///< send info to the user, assumes UTF-8.
    void debug(const char *fmt, ...) override;  ///< send debug info to the user, assumes UTF-8.
    void error(const char *fmt, ...) override;  ///< send error info to the user, assumes UTF-8.
    void warn(const char *fmt, ...)  override;  ///< send warning info to the user, assumes UTF-8.

    constexpr void enableANSIColors(bool enabled) noexcept
    {
        m_enableANSIColors = enabled;
    }

protected:
    bool m_enableANSIColors{false};
};

};