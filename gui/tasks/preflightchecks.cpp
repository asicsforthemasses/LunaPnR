#include "preflightchecks.h"

void Tasks::PreflightChecks::execute(GUI::Database &database, ProgressCallback callback)
{
    bool haveErrors = false;
    m_status.store(Status::RUNNING);

    // TODO: check that we have a valid work dir

    // TODO: check that OpenSTA exists
    auto opensta = database.m_projectSetup.m_openSTALocation;
    if (!ChipDB::fileExists(opensta))
    {
        error("OpenSTA binary cannot be found: configure path in setup\n");
        haveErrors = true;
    }

    // TODO: check that we have a valid technology setup
    if (database.techLib()->getNumberOfLayers() == 0)
    {
        error("No technology layers loaded: add a technology LEF to the project\n");
        haveErrors = true;
    }

    // check that we have a valid cell lib
    if (database.cellLib()->size() <= 1)
    {
        error("No cells loaded: add LIB and/or LEF files to the project\n");
        haveErrors = true;
    }

    // TODO: check that we have a valid modules
    if (database.moduleLib()->size() == 0)
    {
        error("No modules loaded: add verilog files to the project\n");
        haveErrors = true;
    }

    if (haveErrors)
    {
        return;
    }

    // check that we have a top module selected
    auto topModule = database.design().getTopModule();

    if (!topModule)
    {
        if (database.design().m_moduleLib->size() == 1)
        {
            auto moduleIter = database.design().m_moduleLib->begin();
            if (!database.design().setTopModule(moduleIter->name()))
            {
                error("Cannot set top module\n");
                return;
            }
            topModule = database.design().getTopModule();
        }
        else
        {
            error("Cannot deduce top module\n");
            return;
        }
    }    

    // check that the top module has a netlist
    if (!topModule->m_netlist)
    {
        error(Logging::fmt("Module %s does not have a netlist!\n", topModule->name().c_str()));
        return;
    }

    if (database.floorplan()->regionCount() != 0)
    {
        database.floorplan()->clear();
    }

    //FIXME: temporary hack
    auto tempRegion = database.floorplan()->createRegion("DefaultRegion");
    tempRegion->m_site = "CORE";
    tempRegion->m_halo = ChipDB::Margins64{10000,10000,10000,10000};
    tempRegion->m_rect = ChipDB::Rect64({0,0},{100000,100000});

    auto placeRect = tempRegion->getPlacementRect();

    int starty = 0;
    int rowHeight = 10000;
    
    ChipDB::Coord64 ll = placeRect.m_ll + ChipDB::Coord64{0,starty};
    ChipDB::Coord64 ur = ll + ChipDB::Coord64{placeRect.width(), rowHeight};
    size_t skippedRows = 0;
    int numRows = (placeRect.m_ur.m_y - placeRect.m_ll.m_y) / rowHeight;
    info(Logging::fmt("Creating %d rows\n", numRows));

    for(int i=0; i<numRows; i++)
    {
        if (placeRect.contains(ll) && placeRect.contains(ur))
        {
            tempRegion->m_rows.emplace_back();
            auto& row = tempRegion->m_rows.back();

            row.m_region = tempRegion.ptr();
            row.m_rect = ChipDB::Rect64(ll,ur);
        }
        else
        {
            skippedRows++;
        };

        ll += ChipDB::Coord64{0, rowHeight};
        ur += ChipDB::Coord64{0, rowHeight};
    }

    //designPtr->m_floorplan->contentsChanged();

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

    // TODO: check that all pins and pads are placed
    
    //       check we have filler/decap cells
    done();
}
