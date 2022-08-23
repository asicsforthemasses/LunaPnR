// SPDX-FileCopyrightText: 2021-2022 Niels Moseley <asicsforthemasses@gmail.com>
//
// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <vector>
#include "floorplan/floorplan.h"
#include "netlist/netlist.h"
#include "techlib/techlib.h"
#include "celllib/celllib.h"

namespace ChipDB
{

class Design
{
public:
    Design();

    /** clear netlist, celllib, modules and technology information */
    void clear();

    //std::shared_ptr<Netlist>    m_netlist;
    std::shared_ptr<CellLib>    m_cellLib;
    std::shared_ptr<ModuleLib>  m_moduleLib;
    std::shared_ptr<TechLib>    m_techLib;
    std::shared_ptr<Floorplan>  m_floorplan;

    auto createUniqueID()
    {
        return m_uniqueIDCounter++;
    }

    bool setTopModule(const std::string &moduleName);
    
    std::shared_ptr<ChipDB::Module> getTopModule()
    {
        return m_topModule;
    }

protected:
    std::shared_ptr<ChipDB::Module> m_topModule;
    uint32_t m_uniqueIDCounter;
};

};
