#include <iostream>
#include <fstream>
#include <sstream>
#include "luafuncs.h"

#include "common/guihelpers.h"
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
    wrapper->error_print();
}

///> load_verilog(filename : string)
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

    auto wrapper = getLuaWrapper(L);
    wrapper->print("Verilog loaded");

    return 0;
}

///> load_lef(filename : string)
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

    auto wrapper = getLuaWrapper(L);
    wrapper->print("LEF loaded");

    return 0;
}

///> load_lib(filename : string)
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

    auto wrapper = getLuaWrapper(L);
    wrapper->print("LIB loaded");

    return 0;
}

///> load_layers(filename : string)
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
        if (!db->m_layerRenderInfoDB.readJson(buffer.str()))
        {
            reportError(L, "Error parsing Layers file '%s'", filename);
        }
    }
    else
    {
        reportError(L, "Error opening Layers file '%s'", filename);
        return 0;
    }   

    auto wrapper = getLuaWrapper(L);
    wrapper->print("Layers loaded");

    return 0;
}

///> clear() - clear the database
static int clear(lua_State *L)
{
    auto db = getDB(L);

    db->clear();
    auto wrapper = getLuaWrapper(L);
    wrapper->print("Database cleared");

    return 0;
}

///> add_hatch(width : integer, height : integer, pattern : string)
static int add_hatch(lua_State *L)
{
    auto db = getDB(L);

    if (!lua_isinteger(L, 1))
    {
        reportError(L, "Expected an integer for width");
        return 0;
    }

    if (!lua_isinteger(L, 2))
    {
        reportError(L, "Expected an integer for height");
        return 0;
    }

    if (!lua_isstring(L, 3))
    {
        reportError(L, "Expected a string as third parameter");
        return 0;
    }    

    auto width  = lua_tointeger(L, 1);
    auto height = lua_tointeger(L, 2);
    auto str    = lua_tostring(L, 3);

    auto hatchPixmap = GUI::createPixmapFromString(std::string(str), width, height);
    if (hatchPixmap.has_value())
    {
        db->m_hatchLib.m_hatches.push_back(*hatchPixmap);
    }

    auto wrapper = getLuaWrapper(L);
    wrapper->print("Hatch added");

    return 0;
}

///> create_region(regionname : string, x : integer, y : integer, width : integer, height : integer)
static int create_region(lua_State *L)
{
    auto db = getDB(L);

    if (!lua_isstring(L, 1))
    {
        reportError(L, "Param 1: Expected a string for region name");
        return 0;
    }

    if (!lua_isinteger(L, 2))
    {
        reportError(L, "Param 2: Expected an integer for lower left x coordinate");
        return 0;
    }

    if (!lua_isinteger(L, 3))
    {
        reportError(L, "Param 3: Expected an integer for lower left y coordinate");
        return 0;
    }

    if (!lua_isinteger(L, 4))
    {
        reportError(L, "Param 4: Expected an integer for width");
        return 0;
    }    

    if (!lua_isinteger(L, 5))
    {
        reportError(L, "Param 5: Expected an integer for height");
        return 0;
    }

    auto name = lua_tostring(L, 1);
    auto x = lua_tointeger(L, 2);
    auto y = lua_tointeger(L, 3);
    auto width  = lua_tointeger(L, 4);
    auto height = lua_tointeger(L, 5);

    auto region = new ChipDB::Region();
    region->m_rect = ChipDB::Rect64(ChipDB::Coord64{x,y}, ChipDB::Coord64{x+width,y+height});

    if (!db->floorplan().m_regions.add(name, region))
    {
        reportError(L, "Region with name %s already exists!", name);
        delete region;
        return 0;
    }

    auto wrapper = getLuaWrapper(L);
    if (wrapper != nullptr)
    {
        std::stringstream ss;
        ss << "Created region '" << name << "' at " << region->m_rect << "\n";
        wrapper->print(ss);
    }

    db->floorplan().m_regions.contentsChanged();

    return 0;
}

///> create_rows(regionname : string, startY : integer, rowHeight : integer, numberOfRows : integer)
static int create_rows(lua_State *L)
{
    auto db = getDB(L);

    if (!lua_isstring(L, 1))
    {
        reportError(L, "Param 1: Expected a string for region name");
        return 0;
    }

    if (!lua_isinteger(L, 2))
    {
        reportError(L, "Param 2: Expected an integer for start y coordinate");
        return 0;
    }

    if (!lua_isinteger(L, 3))
    {
        reportError(L, "Param 3: Expected an integer for the row height");
        return 0;
    }

    if (!lua_isinteger(L, 4))
    {
        reportError(L, "Param 4: Expected an integer for the number of rows");
        return 0;
    }    

    auto name      = lua_tostring(L, 1);
    auto starty    = lua_tointeger(L, 2);
    auto rowHeight = lua_tointeger(L, 3);
    auto numRows   = lua_tointeger(L, 4);

    auto region = db->floorplan().m_regions.lookup(name);
    if (region == nullptr)
    {
        reportError(L, "Region with name %s does not exists!", name);
        return 0;
    }
    
    ChipDB::Coord64 ll = region->m_rect.m_ll + ChipDB::Coord64{0,starty};
    ChipDB::Coord64 ur = ll + ChipDB::Coord64{region->m_rect.width(), rowHeight};
    for(int i=0; i<numRows; i++)
    {
        region->m_rows.emplace_back();
        auto& row = region->m_rows.back();

        row.m_region = region;
        row.m_rect = ChipDB::Rect64(ll,ur);
        ll += ChipDB::Coord64{0, rowHeight};
        ur += ChipDB::Coord64{0, rowHeight};
    }

    db->floorplan().m_regions.contentsChanged();

    auto wrapper = getLuaWrapper(L);
    wrapper->print("Rows created");

    return 0;
}

///> remove_rows(regionname : string)
static int remove_rows(lua_State *L)
{
    auto db = getDB(L);

    if (!lua_isstring(L, 1))
    {
        reportError(L, "Param 1: Expected a string for region name");
        return 0;
    }

    auto name      = lua_tostring(L, 1);

    auto region = db->floorplan().m_regions.lookup(name);
    if (region == nullptr)
    {
        reportError(L, "Region with name %s does not exists!", name);
        return 0;
    }
    
    db->floorplan().m_regions.contentsChanged();

    region->m_rows.clear();

    auto wrapper = getLuaWrapper(L);
    wrapper->print("Rows removed");

    db->floorplan().m_regions.contentsChanged();

    return 0;
}

///> remove_region(regionname : string)
static int remove_region(lua_State *L)
{
    auto db = getDB(L);

    if (!lua_isstring(L, 1))
    {
        reportError(L, "Param 1: Expected a string for region name");
        return 0;
    }

    auto name      = lua_tostring(L, 1);

    if (!db->floorplan().m_regions.remove(name))
    {
        reportError(L, "Could not regomve region with name %s!", name);
        return 0;
    }

    auto wrapper = getLuaWrapper(L);
    wrapper->print("Region removed");

    db->floorplan().m_regions.contentsChanged();
    return 0;    
}

///> set_region_halo(regionname : string, top : integer, bottom : integer, left : integer, right : integer)
static int set_region_halo(lua_State *L)
{
    auto db = getDB(L);

    if (!lua_isstring(L, 1))
    {
        reportError(L, "Param 1: Expected a string for region name");
        return 0;
    }

    if (!lua_isinteger(L, 2))
    {
        reportError(L, "Param 2: Expected an integer for top halo");
        return 0;
    }

    if (!lua_isinteger(L, 3))
    {
        reportError(L, "Param 3: Expected an integer for bottom halo");
        return 0;
    }

    if (!lua_isinteger(L, 4))
    {
        reportError(L, "Param 4: Expected an integer for left halo");
        return 0;
    }    

    if (!lua_isinteger(L, 5))
    {
        reportError(L, "Param 5: Expected an integer for right halo");
        return 0;
    }

    auto name   = lua_tostring(L, 1);
    auto top    = lua_tointeger(L, 2);
    auto bottom = lua_tointeger(L, 3);
    auto left   = lua_tointeger(L, 4);
    auto right  = lua_tointeger(L, 5);

    auto region = db->floorplan().m_regions.lookup(name);
    if (region == nullptr)
    {
        reportError(L, "Could not find region with name %s!", name);
        return 0;
    }

    // remove the previous halo
    region->m_rect.contract(region->m_halo);

    region->m_halo.m_top = top;
    region->m_halo.m_bottom = bottom;
    region->m_halo.m_left = left;
    region->m_halo.m_right = right;

    // increase the region size by the new halo
    region->m_rect.expand(region->m_halo);

    db->floorplan().m_regions.contentsChanged();

    auto wrapper = getLuaWrapper(L);
    wrapper->print("Region halo updated");

    db->floorplan().m_regions.contentsChanged();

    return 0;    
}

///> place_module(modulename : string, regionname : string)
static int place_module(lua_State *L)
{
    auto db = getDB(L);

    if (!lua_isstring(L, 1))
    {
        reportError(L, "Param 1: Expected a string for module name");
        return 0;
    }

    if (!lua_isstring(L, 2))
    {
        reportError(L, "Param 2: Expected a string for region name");
        return 0;
    }

    auto moduleName = lua_tostring(L, 1);
    auto regionName = lua_tostring(L, 2);
    
    auto *region = db->floorplan().m_regions.lookup(regionName);
    if (region == nullptr)
    {
        reportError(L, "Could not find region with name %s!", regionName);
        return 0;
    }

    auto *mod = db->moduleLib().lookup(moduleName);
    if (mod == nullptr)
    {
        reportError(L, "Could not find module with name %s!", moduleName);
        return 0;
    }


    auto ll = getLogLevel();
    setLogLevel(LOG_VERBOSE);

    if (!LunaCore::QLAPlacer::place(region->getPlacementRect(), mod, nullptr))
    {
        reportError(L, "Placement failed!");
    }
    else
    {
        auto wrapper = getLuaWrapper(L);
        if (wrapper != nullptr)
        {
            wrapper->print("Placement succeeded!\n");
        }     
    }

    setLogLevel(ll);

    db->floorplan().m_regions.contentsChanged();

    return 0;
}

///> place_instance(insname : string, module : string, x : integer, y : integer)
static int place_instance(lua_State *L)
{
    auto db = getDB(L);

    if (!lua_isstring(L, 1))
    {
        reportError(L, "Param 1: Expected a string for instance name");
        return 0;
    }

    if (!lua_isstring(L, 2))
    {
        reportError(L, "Param 2: Expected a string for module name");
        return 0;
    }

    if (!lua_isinteger(L, 3))
    {
        reportError(L, "Param 3: Expected an integer for x location");
        return 0;
    }

    if (!lua_isinteger(L, 4))
    {
        reportError(L, "Param 4: Expected an integer for y location");
        return 0;
    }

    auto insName = lua_tostring(L, 1);
    auto moduleName = lua_tostring(L, 2);
    auto x = lua_tointeger(L,3);
    auto y = lua_tointeger(L,4);

    auto *mod = db->moduleLib().lookup(moduleName);
    if (mod == nullptr)
    {
        reportError(L, "Could not find module with name %s!", moduleName);
        return 0;
    }

    if (!mod->m_netlist)
    {
        reportError(L, "Module %s has no instances!", moduleName);
        return 0;        
    }

    auto *ins = mod->m_netlist.get()->m_instances.lookup(insName);
    if (ins == nullptr)
    {
        reportError(L, "Could not find instance with name %s!", insName);
        return 0;
    }

    ins->m_pos.m_x = x;
    ins->m_pos.m_y = y;
    ins->m_placementInfo = ChipDB::PlacementInfo::PLACEDANDFIXED;

    db->floorplan().m_regions.contentsChanged();

    return 0;
}

///> set_toplevel_module(module : string)
static int set_toplevel_module(lua_State *L)
{
    auto db = getDB(L);

    if (!lua_isstring(L, 1))
    {
        reportError(L, "Param 1: Expected a string for module name");
        return 0;
    }

    auto moduleName = lua_tostring(L, 1);

    if (!db->design().setTopModule(moduleName))
    {
        reportError(L, "Could not find module with name %s!", moduleName);
        return 0;        
    }

    return 0;
};

///> write_placement(module : string)
static int write_placement(lua_State *L)
{
    auto db = getDB(L);

    if (!lua_isstring(L, 1))
    {
        reportError(L, "Param 1: Expected a string for module name");
        return 0;
    }

    if (!lua_isstring(L, 2))
    {
        reportError(L, "Param 2: Expected a string for file name");
        return 0;
    }

    auto moduleName = lua_tostring(L, 1);

    auto mod = db->moduleLib().lookup(moduleName);

    if (mod == nullptr)
    {
        reportError(L, "Could not find module with name %s!", moduleName);
        return 0;        
    }

    auto fileName = lua_tostring(L,2);

    std::ofstream ofile(fileName);
    if (!ofile.good())
    {
        reportError(L, "Could not write placement file%s!", fileName);
        return 0;
    }

    LunaCore::NetlistTools::writePlacementFile(ofile, mod->m_netlist.get());

    return 0;
};


///> write_density_bitmap(modulename : string, regionname : string, bitmapname : string)
static int write_density_bitmap(lua_State *L)
{
    auto db = getDB(L);

    if (!lua_isstring(L, 1))
    {
        reportError(L, "Param 1: Expected a string for module name");
        return 0;
    }

    if (!lua_isstring(L, 2))
    {
        reportError(L, "Param 2: Expected a string for region name");
        return 0;
    }

    if (!lua_isstring(L, 3))
    {
        reportError(L, "Param 3: Expected a string for file name");
        return 0;
    }

    auto moduleName = lua_tostring(L, 1);
    auto regionName = lua_tostring(L, 2);
    auto fileName   = lua_tostring(L, 3);
    
    auto *region = db->floorplan().m_regions.lookup(regionName);
    if (region == nullptr)
    {
        reportError(L, "Could not find region with name %s!", regionName);
        return 0;
    }

    auto *mod = db->moduleLib().lookup(moduleName);
    if (mod == nullptr)
    {
        reportError(L, "Could not find module with name %s!", moduleName);
        return 0;
    }

    auto bitmap = LunaCore::QPlacer::createDensityBitmap(mod->m_netlist.get(), region,
        10000,10000);

    if (!bitmap)
    {
        reportError(L, "Could not create bitmap!");
        return 0;
    }

    std::ofstream ofile(fileName);
    if (!ofile.good())
    {
        reportError(L, "Could not create bitmap file!");
        return 0;        
    }

    LunaCore::QPlacer::writeToPGM(ofile, bitmap);
    return 0;
}

void Lua::registerFunctions(lua_State *L)
{
    lua_register(L, "clear", clear);
    lua_register(L, "add_hatch", add_hatch);
    lua_register(L, "load_verilog", load_verilog);
    lua_register(L, "load_lef", load_lef);
    lua_register(L, "load_lib", load_lib);
    lua_register(L, "load_layers", load_layers);
    lua_register(L, "create_region", create_region);
    lua_register(L, "remove_region", remove_region);
    lua_register(L, "set_region_halo", set_region_halo);
    lua_register(L, "create_rows", create_rows);
    lua_register(L, "remove_rows", remove_rows);
    lua_register(L, "place_module", place_module);
    lua_register(L, "place_instance", place_instance);
    lua_register(L, "set_toplevel_module", set_toplevel_module);
    lua_register(L, "write_placement", write_placement);
    lua_register(L, "write_density_bitmap", write_density_bitmap);
}
