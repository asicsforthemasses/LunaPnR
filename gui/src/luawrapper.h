/*
    LunaPnR Source Code

    SPDX-License-Identifier: GPL-3.0-only
    SPDX-FileCopyrightText: 2022 Niels Moseley <asicsforthemasses@gmail.com>
*/


#pragma once
#include "lua.hpp"

#include <type_traits>
#include <string>
#include <unordered_map>
#include <QString>

#include "../console/mmconsole.h"
#include "../common/database.h"

namespace GUI
{

struct LuaWrapperObject
{
    const char *m_clsname;
};

class LuaWrapper
{
public:
    LuaWrapper(MMConsole *console, std::shared_ptr<Database> db);
    virtual ~LuaWrapper();

    bool run(const char *txt);
    bool run(const QString &txt);
    bool run(const std::string &txt);

    /** dump the LUA stack to a stream */
    void dumpStack(std::ostream &os);

    /** replacement print function for LUA */
    void print();

    /** print function to call in lua C functions */
    void print(const std::string &txt);
    void print(const std::stringstream &ss);

    /** replacement print function for LUA, but with error colours */
    void error_print();

    void clearConsole();

protected:
    MMConsole *m_console;
    std::shared_ptr<Database> m_db;
    lua_State *m_L;
};

};
