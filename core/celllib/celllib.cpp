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

//ModuleLib::~ModuleLib()
//{
//}

void ModuleLib::clear()
{
    m_modules.clear();
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
    m_modules.add(name, newModule);
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
