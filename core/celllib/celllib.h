#pragma once

#include <string>
#include <vector>
#include "pin.h"
#include "common/namedstorage.h"

namespace ChipDB
{

class Net;  // predeclaration

class Cell
{
public:
    std::string             m_name;
    std::vector<PinInfo>    m_pins;

    double          m_area;         // area in um^2
    double          m_leakagePower; // in Watts
    Coord64         m_size;         // size in nm

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

    Module* at(size_t index);
    const Module* at(size_t index) const;
    Module* lookup(const std::string &name) const;
    Module* createModule(const std::string &name);
};

};