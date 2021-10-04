#pragma once
#include "lua.hpp"

#include <string>
#include <QString>

#include "../console/mmconsole.h"

namespace GUI
{

class LuaWrapper
{
public:
    LuaWrapper(MMConsole *console);
    virtual ~LuaWrapper();

    bool run(const char *txt);
    bool run(const QString &txt);
    bool run(const std::string &txt);

    void dumpStack(std::ostream &os);

    void print();

protected:
    MMConsole *m_console;
    lua_State *m_L;
};

};