#pragma once
#include "lua.hpp"

#include <string>
#include <QString>

#include "../console/mmconsole.h"
#include "../common/database.h"

namespace GUI
{

class LuaWrapper
{
public:
    LuaWrapper(MMConsole *console, Database &db);
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
    Database &m_db;    
    lua_State *m_L;
};

};
