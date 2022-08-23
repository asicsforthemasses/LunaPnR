// SPDX-FileCopyrightText: 2021-2022 Niels Moseley <asicsforthemasses@gmail.com>
//
// SPDX-License-Identifier: GPL-3.0-only

#include <chrono>
#include "place.h"

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

    // FIXME: for now use the first region to place the top module
    // check there is a valid floorplan
    if (database.floorplan()->regionCount() == 0)
    {
        error("No regions defined in floorplan!\n");
        return;
    }

    for(auto pin : topModule->m_pins)
    {
        auto pinInstance = topModule->m_netlist->lookupInstance(pin->m_name);
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

    auto regionKeyPair = *(database.floorplan()->begin());

    bool ok = LunaCore::QLAPlacer::place(*regionKeyPair.rawPtr(), 
        *topModule->m_netlist.get(), nullptr);

    if (!ok)
    {
        error("Placement failed!\n");
        return;
    }

    done();
}
