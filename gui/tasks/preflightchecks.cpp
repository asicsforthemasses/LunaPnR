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

    // TODO: check that all pins and pads are placed
    //       check there is a valid floorplan
    //       check we have filler/decap cells
    done();
}
