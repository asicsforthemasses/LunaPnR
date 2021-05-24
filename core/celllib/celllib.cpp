#include <algorithm>
#include "celllib.h"
#include "netlist/netlist.h"

using namespace ChipDB;

PinInfo& Cell::createPin(const std::string &name)
{
    auto iter = std::find_if(m_pins.begin(), m_pins.end(),
        [name](const auto p)
        {
            return p.m_name == name;
        }
    );

    if (iter == m_pins.end())
    {
        // pin does not exist, so we create it
        m_pins.emplace_back(PinInfo());
        m_pins.back().m_name = name;
        return m_pins.back();
    }
    else
    {
        return *iter;
    }
}

PinInfo* Cell::lookupPin(const std::string &name)
{
    auto iter = std::find_if(m_pins.begin(), m_pins.end(),
    [name](const auto p)
        {
            return p.m_name == name;
        }
    );

    if (iter == m_pins.end())
    {
        // pin does not exist
        return nullptr;
    }
    else
    {
        return &(*iter);
    }    
}

ssize_t Cell::lookupPinIndex(const std::string &name) const
{
    for(size_t i=0; i<m_pins.size(); i++)
    {
        if (m_pins[i].m_name == name)
        {
            return i;
        }
    }

    return -1;
}

bool Module::addInstance(const std::string &insName, AbstractInstance* insPtr)
{
    return m_instances.add(insName, insPtr);
}

Net* Module::createNet(const std::string &netName)
{
    // if the net already exists, return that one.
    auto myNet = m_nets.lookup(netName);
    if (myNet != nullptr)
        return myNet;

    myNet = new Net;
    myNet->m_name = netName;
    m_nets.add(netName, myNet);

    return myNet;
}


CellLib::~CellLib()
{
    for(auto ptr : m_cells)
    {
        delete ptr;
    }
}

Cell* CellLib::createCell(const std::string &name)
{
    auto cell = lookup(name);
    if (cell != nullptr)
    {
        return cell;
    }   

    cell = new Cell();
    cell->m_name = name;

    m_cells.add(name, cell);
    return cell;
}

Cell* CellLib::lookup(const std::string &name) const
{
    auto iter = std::find_if(m_cells.begin(), m_cells.end(),
        [name](auto ptr)
        {
            return ptr->m_name == name;
        }
    );

    if (iter == m_cells.end())
        return nullptr;

    return *iter;
}

ModuleLib::~ModuleLib()
{
    for(auto ptr : m_modules)
    {
        delete ptr;
    }
}

Module* ModuleLib::lookup(const std::string &name) const
{
    auto iter = std::find_if(m_modules.begin(), m_modules.end(),
        [name](auto ptr)
        {
            return ptr->m_name == name;
        }
    );

    if (iter == m_modules.end())
        return nullptr;

    return *iter;    
}

Module* ModuleLib::createModule(const std::string &name)
{
    auto newModule = new Module();
    newModule->m_name = name;
    m_modules.push_back(newModule);
    return newModule;
}

Module* ModuleLib::at(size_t index)
{
    return m_modules.at(index);
}

const Module* ModuleLib::at(size_t index) const
{
    return m_modules.at(index);
}
