/*
  LunaPnR Source Code
  
  SPDX-License-Identifier: GPL-3.0-only
  SPDX-FileCopyrightText: 2022 Niels Moseley <asicsforthemasses@gmail.com>
*/


// see: https://www.fatalerrors.org/a/lua-and-c-c-interoperation.html

#include "luawrapper.h"
#include "../luafuncs/luafuncs.h"
#include <iostream>
#include <sstream>

using namespace GUI;

template<class T> 
struct PushObject
{
    static T* checkObj(lua_State* lua)
    {
        auto userData = luaL_checkudata(lua, 1, T::className);
        if (userData == nullptr)
        {
            luaL_typeerror(lua, 1, T::className);
            return nullptr; // we will never get here.
        }
        return *reinterpret_cast<T**>(userData);        
    }

    static constexpr auto garbageCollect = [](lua_State* lua) -> int
    {
        T *obj = checkObj(lua);
        if (obj != nullptr)
        {
            obj->gc(lua);
        }
        return 0;
    };

    static constexpr auto call = [](lua_State* lua) -> int
    {
        T *obj = checkObj(lua);
        if (obj != nullptr)
        {
            return obj->call(lua);
        }     
        return 0;
    }; 

    static constexpr auto index = [](lua_State* lua) -> int
    {
        T *obj = checkObj(lua);
        if (obj != nullptr)
        {
            return obj->index(lua);
        }     
        return 0;
    };    

    static constexpr auto newindex = [](lua_State* lua) -> int
    {
        T *obj = checkObj(lua);
        if (obj != nullptr)
        {
            return obj->newindex(lua);
        }     
        return 0;
    };
};


template<class T>
void toLua(lua_State *L, T* obj)
{   
    auto ptrptr = static_cast<T**>(lua_newuserdata(L, sizeof(T*)));
    *ptrptr = obj;

    luaL_getmetatable(L, T::className);
    if (lua_isnil(L, -1))
    {
        lua_pop(L,-1);

        std::cout << "Metatable created!\n";
        luaL_newmetatable(L, T::className);

        lua_pushcfunction(L, PushObject<T>::call);
        lua_setfield( L, -2, "__call");

        lua_pushcfunction(L, PushObject<T>::index);
        lua_setfield( L, -2, "__index");
        
        lua_pushcfunction(L, PushObject<T>::newindex);
        lua_setfield( L, -2, "__newindex");
        
        lua_pushcfunction(L, PushObject<T>::garbageCollect);
        lua_setfield( L, -2, "__gc");
    }
    
    lua_setmetatable(L, -2);
}


static int wrapper_print(lua_State *L)
{
    lua_getglobal(L, "luawrapperobj");
    auto wrapperPtr = (LuaWrapper*)lua_touserdata(L,-1);
    lua_pop(L,1);

    if (wrapperPtr != nullptr)
    {
        wrapperPtr->print();
    }
    else
    {
        // print to OS console, if there is one.
        int count = lua_gettop(L);
        for(int idx=1; idx<=count; idx++)
        {
            const char *str = lua_tostring(L, idx);
            std::cout << str << "\n";
        }
    }

    return 0;
}

#if 0
struct Fart
{
    static constexpr const char* className = "Fart";

    void gc(lua_State *L) { return; }
    int call(lua_State *L) { std::cout << "Called!\n"; return 0; }
    int index(lua_State *L) { return 0; }
    int newindex(lua_State *L) { return 0; }
};

Fart myFart;

static int fart(lua_State *L)
{
    toLua(L, &myFart);
    return 1;
}
#endif

LuaWrapper::LuaWrapper(MMConsole *console, std::shared_ptr<Database> db) : m_console(console), m_db(db)
{
    m_L = luaL_newstate();
    luaL_openlibs(m_L);

    // replace the LUA print function with our own
    lua_register(m_L,"print", wrapper_print);

    //FIXME: this pointer should not be mutable by the VM..
    // expose the LuaWrapper object to the LUA VM
    lua_pushlightuserdata(m_L, this);
    lua_setglobal(m_L, "luawrapperobj");

    //FIXME: this pointer should not be mutable by the VM..
    // expose the Database object to the LUA VM
    lua_pushlightuserdata(m_L, m_db.get());
    lua_setglobal(m_L, "databaseobj");

    Lua::registerFunctions(m_L);
    //lua_register(m_L, "fart", &fart);
}

LuaWrapper::~LuaWrapper()
{
    lua_close(m_L);
}

void LuaWrapper::error_print()
{
    int count = lua_gettop(m_L);
    for(int idx=1; idx<=count; idx++)
    {
        const char *str = lua_tostring(m_L, idx);
        if (m_console != nullptr)
        {
            std::stringstream ss;
            ss << str;

            m_console->print(ss, GUI::MMConsole::PrintType::Error);
        }
        else
        {
            std::cout << "LUA: " << str << "\n";
        }        
    }        
}

void LuaWrapper::print()
{
    int count = lua_gettop(m_L);
    for(int idx=1; idx<=count; idx++)
    {
        const char *str = lua_tostring(m_L, idx);
        if (m_console != nullptr)
        {
            std::stringstream ss;
            ss << str;

            m_console->print(ss, (idx < count) ? GUI::MMConsole::PrintType::Partial : GUI::MMConsole::PrintType::Complete);
        }
        else
        {
            std::cout << "LUA: " << str << "\n";
        }        
    }    
}

void LuaWrapper::print(const std::string &txt)
{
    if (m_console != nullptr)
    {
        m_console->print(txt, GUI::MMConsole::PrintType::Complete);
    }
}

void LuaWrapper::print(const std::stringstream &ss)
{
    if (m_console != nullptr)
    {
        m_console->print(ss, GUI::MMConsole::PrintType::Complete);
    }
}

void LuaWrapper::clearConsole()
{
    if (m_console != nullptr)
    {
        m_console->clear();
    }    
}

bool LuaWrapper::run(const char *txt)
{
    auto result = luaL_dostring(m_L, txt);
    if (result != 0)
    {
        if (m_console != nullptr)
        {
            std::stringstream ss;
            ss << "ERROR: " << lua_tostring(m_L, -1) << "\n";
            m_console->print(ss, GUI::MMConsole::PrintType::Error);
        }
        else
        {
            std::cout << "LUA ERROR: " << lua_tostring(m_L, -1) << "\n";
        }
        lua_pop(m_L,1);
        return false;
    }

    return true;
}

bool LuaWrapper::run(const QString &txt)
{
    return run(txt.toStdString());
}

bool LuaWrapper::run(const std::string &txt)
{
    return run(txt.c_str());
}

void LuaWrapper::dumpStack(std::ostream &os)
{
    os << "Stack dump:" << "\n";
    auto idx = lua_gettop(m_L);
    while(idx)
    {   
        auto luaType = lua_type(m_L, idx);
        switch(luaType)
        {
        case LUA_TSTRING:
            os << idx << " '" << lua_tostring(m_L, idx) << "'\n";
            break;
        case LUA_TBOOLEAN:
            os << idx << " '" << (lua_toboolean(m_L, idx) ? "TRUE" : "FALSE") << "'\n";
            break;        
        case LUA_TNUMBER:
            os << idx << " '" << lua_tonumber(m_L, idx) << "'\n";
            break;
        case LUA_TFUNCTION:
            os << idx << " FUNCTION\n";
            break;
        case LUA_TTABLE:
            os << idx << " TABLE\n";
            break;
        case LUA_TLIGHTUSERDATA:
            os << idx << " LIGHT USERDATA\n";
            break;
        case LUA_TUSERDATA:
            os << idx << " USERDATA\n";
            break;
        case LUA_TTHREAD:
            os << idx << " THREAD\n";
            break;            
        default:
            os << idx << " " << lua_typename(m_L, luaType) << "\n";
            break;
        }
        idx--;
    }
}
