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

    Module* lookup(const std::string &name) const;
    Module* create(const std::string &name);
};

};