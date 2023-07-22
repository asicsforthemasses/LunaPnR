// SPDX-FileCopyrightText: 2021-2022,2023 Niels Moseley <asicsforthemasses@gmail.com>
//
// SPDX-License-Identifier: GPL-3.0-only

#include "design.h"

using namespace ChipDB;

Design::Design()
{
    m_techLib = std::make_shared<TechLib>();
    m_cellLib = std::make_shared<CellLib>();
    m_moduleLib = std::make_shared<ModuleLib>();
    m_floorplan = std::make_shared<Floorplan>();
}

void Design::clear()
{
    //m_netlist->clear();
    m_techLib->clear();
    m_cellLib->clear();
    m_moduleLib->clear();
    m_floorplan->clear();

    m_topModule = nullptr;
    m_uniqueIDCounter = 0;
}

bool Design::setTopModule(const std::string &moduleName)
{
    auto moduleKeyObjPair = m_moduleLib->lookupModule(moduleName);
    if (moduleKeyObjPair.isValid())
    {
        m_topModule = moduleKeyObjPair.ptr();
        return true;
    }
    return false;
}
