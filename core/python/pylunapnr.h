// SPDX-FileCopyrightText: 2021-2025 Niels Moseley <asicsforthemasses@gmail.com>
//
// SPDX-License-Identifier: GPL-3.0-only

#pragma once
#include <type_traits>
#include <functional>
#include <string>
#include "database/database.h"

namespace Scripting
{

class Python
{
public:
    Python(ChipDB::Design *design);
    virtual ~Python();

    void init();

    virtual bool preInitHook() { return true; };
    virtual bool postInitHook() { return true; };

    bool executeScript(const std::string &code);
    bool executeScriptFile(const std::string &fileName);

    void setConsoleRedirect(std::function<void(const char *, ssize_t)> stdoutFunc,
        std::function<void(const char *, ssize_t)> stderrFunc);

protected:
    bool            m_initCalled = false;
    ChipDB::Design *m_design;
};

};