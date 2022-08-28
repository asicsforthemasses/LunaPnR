// SPDX-FileCopyrightText: 2021-2022 Niels Moseley <asicsforthemasses@gmail.com>
//
// SPDX-License-Identifier: GPL-3.0-only

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
    createInputPinCell();
    createOutputPinCell();
    createIOPinCell();
}

KeyObjPair<Cell> CellLib::createCell(const std::string &name)
{
    // return a cell of the same name if it exists
    auto cellObjKey = m_cells[name];
    if (cellObjKey.isValid())
    {
        return cellObjKey;
    }

    auto cellObjKeyOptional = m_cells.add(std::make_shared<Cell>(name));
    if (!cellObjKeyOptional)
    {
        return KeyObjPair<Cell>{};
    }

    return cellObjKeyOptional.value();
}

KeyObjPair<Cell> CellLib::lookupCell(const std::string &name) const
{
    return m_cells[name];
}

KeyObjPair<Cell> CellLib::lookupCell(const std::string &name)
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

void CellLib::createOutputPinCell()
{
    auto outPinCell = createCell("__OUTPIN");
    outPinCell->m_size = {0,0};
    outPinCell->m_area = 0;
    outPinCell->m_leakagePower = 0;
    auto inPin  = outPinCell->m_pins.createPin("A");    // inner connection
    inPin->m_iotype  = IOType::INPUT;
}

void CellLib::createInputPinCell()
{
    auto outPinCell = createCell("__INPIN");
    outPinCell->m_size = {0,0};
    outPinCell->m_area = 0;
    outPinCell->m_leakagePower = 0;
    auto outPin = outPinCell->m_pins.createPin("Y");    // inner connection first
    outPin->m_iotype = IOType::OUTPUT;
}

void CellLib::createIOPinCell()
{
    auto ioPinCell = createCell("__IOPIN");
    ioPinCell->m_size = {0,0};
    ioPinCell->m_area = 0;
    ioPinCell->m_leakagePower = 0;
    auto ioPin  = ioPinCell->m_pins.createPin("IO");    // inner connection first
    ioPin->m_iotype  = IOType::IO;
}

void CellLib::addListener(INamedStorageListener *listener)
{
    m_cells.addListener(listener);
}

void CellLib::removeListener(INamedStorageListener *listener)
{
    m_cells.removeListener(listener);
}

// **********************************************************************
//   ModuleLib
// **********************************************************************

void ModuleLib::clear()
{
    m_modules.clear();
}

KeyObjPair<Module> ModuleLib::lookupModule(const std::string &name) const
{
    return m_modules[name];
}

KeyObjPair<Module> ModuleLib::lookupModule(const std::string &name)
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

KeyObjPair<Module> ModuleLib::createModule(const std::string &name)
{
    auto result = m_modules.add(std::make_shared<Module>(name));
    if (result)
    {
        return result.value();
    }

    return KeyObjPair<Module>{};
}

bool ModuleLib::removeModule(ObjectKey key)
{
    return m_modules.remove(key);
}

bool ModuleLib::removeModule(const std::string &name)
{
    return m_modules.remove(name);
}

void ModuleLib::addListener(INamedStorageListener *listener)
{
    m_modules.addListener(listener);
}

void ModuleLib::removeListener(INamedStorageListener *listener)
{
    m_modules.removeListener(listener);
}
