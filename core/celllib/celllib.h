#pragma once

#include <string>
#include <vector>
#include "common/dbtypes.h"
#include "common/namedstorage.h"
#include "pin.h"

namespace ChipDB
{

class Net;  // predeclaration

class Cell
{
public:
    Cell() : m_area(0), m_leakagePower(0)
    {
    }

    std::string             m_name;
    std::vector<PinInfo>    m_pins;

    double          m_area;         // area in um^2
    double          m_leakagePower; // in Watts
    Coord64         m_size;         // size in nm
    Coord64         m_offset;       // placement offset in nm

    SymmetryFlags   m_symmetry;
    std::string     m_site;

    PinInfo&    createPin(const std::string &name);
    PinInfo*    lookupPin(const std::string &name);
    ssize_t     lookupPinIndex(const std::string &name) const;

    virtual bool isModule() const
    {
        return false;
    };
};

class Module : public Cell
{
public:
    virtual bool isModule() const override
    {
        return true;
    }

    bool addInstance(const std::string &insName, AbstractInstance* insPtr);
    Net* createNet(const std::string &netName);

    NamedStorage<AbstractInstance*> m_instances;
    NamedStorage<Net*>  m_nets;
};

class CellLib
{
public:
    virtual ~CellLib();

    NamedStorage<Cell*> m_cells;

    size_t size() const
    {
        return m_cells.size();
    };

    auto begin() const
    {
        return m_cells.begin();
    }

    auto end() const
    {
        return m_cells.end();
    }

    Cell* createCell(const std::string &name);
    Cell* lookup(const std::string &name) const;
};

class ModuleLib
{
public:
    virtual ~ModuleLib();

    std::vector<Module*> m_modules;

    size_t size() const
    {
        return m_modules.size();
    };

    auto begin() const
    {
        return m_modules.begin();
    }

    auto end() const
    {
        return m_modules.end();
    }

    Module* at(size_t index);
    const Module* at(size_t index) const;
    Module* lookup(const std::string &name) const;
    Module* createModule(const std::string &name);
};

};