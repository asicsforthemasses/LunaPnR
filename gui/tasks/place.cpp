// SPDX-FileCopyrightText: 2021-2024 Niels Moseley <asicsforthemasses@gmail.com>
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

    if (database.floorplan()->coreSize().isNullSize())
    {
        error("Core area is not defined; create a floorplan!\n");
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

#if 0
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
#endif

    info("Using CellPlacer2\n");
    LunaCore::CellPlacer2::Placer placer;
    if (!placer.place(*netlist, *database.floorplan(), 20, 10))
    {
        error("Placement failed\n");
        return;
    }

    auto cellLib = database.cellLib();
    if (!cellLib)
    {
        error("No cell library defined in database\n");
        return;
    }

    // write terminal placement / nets to file
    std::ofstream netFile("netpos.txt");
    if (netFile.good())
    {
        if (!LunaCore::TXT::write(netFile, netlist))
        {
            warning("  failed to write netpos.txt\n");
        }
    }
    else
    {
        warning("  failed to open netpos.txt for writing\n");
    }

#if 0
    info("Placing fillers..\n");
    LunaCore::FillerHandler fillerHandler(*cellLib.get());

    fillerHandler.addFillerByName(*database.cellLib().get() ,"FILL");
    if (!fillerHandler.placeFillers(database.design(), *firstRegion, *netlist))
    {
        warning("  failed!\n");
    }
    else
    {
        info("  succeeded!\n");
    }
#endif

    // temporarily save the placement as a DEF file
    std::ofstream deffile("placement.def");
    LunaCore::DEF::WriterOptions options;
    options.exportDecap   = false;
    options.exportFillers = false;
    if (LunaCore::DEF::write(deffile, topModule, options))
    {
        info("  DEF file written.\n");
    }

    info("Placement done.\n");
    done();
}
