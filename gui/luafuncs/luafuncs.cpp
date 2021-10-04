#include <iostream>
#include <fstream>
#include <sstream>
#include "luafuncs.h"

#include "common/database.h"
#include "src/luawrapper.h"

static GUI::Database* getDB(lua_State *L)
{
    lua_getglobal(L, "databaseobj");
    auto databasePtr = (GUI::Database*)lua_touserdata(L,-1);
    lua_pop(L,1);    

    if (databasePtr == nullptr)
    {
        luaL_error(L, "cannot access database object");
    }

    return databasePtr;
}

static GUI::LuaWrapper* getLuaWrapper(lua_State *L)
{
    lua_getglobal(L, "luawrapperobj");
    auto wrapperPtr = (GUI::LuaWrapper*)lua_touserdata(L,-1);
    lua_pop(L,1);    

    if (wrapperPtr == nullptr)
    {
        luaL_error(L, "cannot access lua wrapper object");
    }

    return wrapperPtr;
}

static void reportError(lua_State *L, const char *fmt, ...)
{
    va_list argp;
    va_start(argp, fmt);
    luaL_where(L, 1);
    lua_pushvfstring(L, fmt, argp);
    va_end(argp);
    lua_concat(L, 2);

    auto wrapper = getLuaWrapper(L);
    wrapper->print();
}

static int load_verilog(lua_State *L)
{
    auto db = getDB(L);

    auto filename = lua_tostring(L,-1);
    if (filename == nullptr)
    {
        reportError(L, "expected a filename argument");
        return 0;
    }

    std::ifstream verilogFile(filename);
    if (!verilogFile.good())
    {
        reportError(L, "cannot open verilog file '%s'", filename);
        return 0;
    }

    if (!ChipDB::Verilog::Reader::load(&db->design(), verilogFile))
    {
        reportError(L, "Error parsing verilog file '%s'", filename);
    }

    return 0;
}

static int load_lef(lua_State *L)
{
    auto db = getDB(L);

    auto filename = lua_tostring(L,-1);
    if (filename == nullptr)
    {
        reportError(L, "expected a filename argument");
        return 0;
    }

    std::ifstream lefFile(filename);
    if (!lefFile.good())
    {
        reportError(L, "cannot open LEF file '%s'", filename);
        return 0;
    }

    if (!ChipDB::LEF::Reader::load(&db->design(), lefFile))
    {
        reportError(L, "Error parsing LEF file '%s'", filename);
        return 0;
    }

    return 0;
}

static int load_lib(lua_State *L)
{
    auto db = getDB(L);

    auto filename = lua_tostring(L,-1);
    if (filename == nullptr)
    {
        reportError(L, "expected a filename argument");
        return 0;
    }

    std::ifstream libFile(filename);
    if (!libFile.good())
    {
        reportError(L, "cannot open Liberty file '%s'", filename);
        return 0;
    }

    if (!ChipDB::Liberty::Reader::load(&db->design(), libFile))
    {
        reportError(L, "Error parsing Liberty file '%s'", filename);
        return 0;
    }

    return 0;
}

static int load_layers(lua_State *L)
{
    auto db = getDB(L);

    auto filename = lua_tostring(L,-1);
    if (filename == nullptr)
    {
        reportError(L, "expected a filename argument");
        return 0;
    }

    std::ifstream layersFile(filename);
    if (layersFile.good())
    {
        std::stringstream buffer;
        buffer << layersFile.rdbuf();
        db->m_layerRenderInfoDB.readJson(buffer.str());
    }
    else
    {
        reportError(L, "Error parsing Layers file '%s'", filename);
        return 0;
    }   

    return 0;
}

void Lua::registerFunctions(lua_State *L)
{
    lua_register(L, "load_verilog", load_verilog);
    lua_register(L, "load_lef", load_lef);
    lua_register(L, "load_lib", load_lib);
    lua_register(L, "load_layers", load_layers);
}
