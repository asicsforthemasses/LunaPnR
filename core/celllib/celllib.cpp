#include <algorithm>
#include "celllib.h"

using namespace ChipDB;

// **********************************************************************
//   CellLib
// **********************************************************************

void CellLib::clear()
{
    m_cells.clear();
    createNetConCell();
}

CreateResult<Cell> CellLib::createCell(const std::string &name)
{
    CreateResult<Cell> result;

    auto newCell = std::make_shared<Cell>(name);
    auto cellObjKey = m_cells.add(newCell);
    if (!cellObjKey)
    {
        return result;
    }

    result.m_key = cellObjKey.value();
    result.m_obj = newCell;

    return result;
}

const std::shared_ptr<Cell> CellLib::lookupCell(const std::string &name) const
{
    return m_cells[name];
}

std::shared_ptr<Cell> CellLib::lookupCell(const std::string &name)
{
    return m_cells[name];
}

const std::shared_ptr<Cell> CellLib::lookupCell(ObjectKey key) const
{
    return m_cells[key];
}

std::shared_ptr<Cell> CellLib::lookupCell(ObjectKey key)
{
    return m_cells[key];
}

void CellLib::createNetConCell()
{
    auto netConCell = createCell("__NETCON");
    netConCell->m_size = {0,0};
    netConCell->m_area = 0;
    netConCell->m_leakagePower = 0;
    auto inPin  = netConCell->m_pins.createPin("A");
    auto outPin = netConCell->m_pins.createPin("Y");
    inPin->m_iotype  = IOType::INPUT;
    outPin->m_iotype = IOType::OUTPUT;
}


// **********************************************************************
//   ModuleLib
// **********************************************************************

void ModuleLib::clear()
{
    m_modules.clear();
}

const std::shared_ptr<Module> ModuleLib::lookupModule(const std::string &name) const
{
    return m_modules[name];
}

std::shared_ptr<Module> ModuleLib::lookupModule(const std::string &name)
{
    return m_modules[name];
}

const std::shared_ptr<Module> ModuleLib::lookupModule(ObjectKey key) const
{
    return m_modules[key];
}

std::shared_ptr<Module> ModuleLib::lookupModule(ObjectKey key)
{
    return m_modules[key];
}

CreateResult<Module> ModuleLib::createModule(const std::string &name)
{
    CreateResult<Module> result;

    auto newModule = std::make_shared<Module>(name);
    auto moduleObjKey = m_modules.add(newModule);
    if (!moduleObjKey)
    {
        return result;
    }

    result.m_key = moduleObjKey.value();
    result.m_obj = newModule;

    return result;
}

bool ModuleLib::removeModule(ObjectKey key)
{
    m_modules.remove(key);
}

bool ModuleLib::removeModule(const std::string &name)
{
    m_modules.remove(name);
}
