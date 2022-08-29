// SPDX-FileCopyrightText: 2021-2022 Niels Moseley <asicsforthemasses@gmail.com>
//
// SPDX-License-Identifier: GPL-3.0-only

#include <chrono>
#include "place.h"
#include "lunacore.h"

void Tasks::Place::execute(GUI::Database &database, ProgressCallback callback)
{
    m_status.store(Status::RUNNING);

    auto topModule = database.design().getTopModule();

    if (!topModule)
    {
        error("Top module not set\n");
        return;
    }    

    if (!topModule->m_netlist)
    {
        error("Top module has no netlist\n");
        return;        
    }

    auto netlist = topModule->m_netlist;

    // FIXME: for now use the first region to place the top module
    // check there is a valid floorplan
    if (database.floorplan()->regionCount() == 0)
    {
        error("No regions defined in floorplan!\n");
        return;
    }

    for(auto pin : topModule->m_pins)
    {
        auto pinInstance = netlist->lookupInstance(pin->m_name);
        if (!pinInstance.isValid())
        {
            error(Logging::fmt("Module %s does not have a pin instance corresponding to pin %s!\n", topModule->name().c_str(), pin->m_name.c_str()));
            return;            
        }
        if (!pinInstance->isPlaced())
        {
            error(Logging::fmt("Pin %s of module %s has not been placed!\n", pin->m_name.c_str(), topModule->name().c_str()));
            return;
        }
    }

    auto regionIter = database.floorplan()->begin();
    if (regionIter == database.floorplan()->end())
    {
        error("No regions defined in floorplan!\n");
        return;
    }
    
    auto firstRegion = *regionIter;
    if (!firstRegion.isValid())
    {
        error("First region in floorplan is invalid!\n");
        return;
    }

#if 1
    bool ok = LunaCore::QLAPlacer::place(*firstRegion.rawPtr(), 
        *topModule->m_netlist.get(), nullptr);
#else
    LunaCore::CellPlacer2::Placer placer;
    placer.place(*netlist, *firstRegion, 20, 10);
    bool ok = true;
    //bool ok = LunaCore::CellPlacer2::place(database, )
#endif
    if (!ok)
    {
        error("Placement failed!\n");
        return;
    }

    done();
}
