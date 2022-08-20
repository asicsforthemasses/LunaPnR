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
    auto regionIter = database.floorplan()->begin();
    if (regionIter == database.floorplan()->end())
    {
        error("No regions defined\n");
        return;
    }

    auto regionKeyPair = *regionIter;

    bool ok = LunaCore::QLAPlacer::place(*regionKeyPair.rawPtr(), 
        *topModule->m_netlist.get(), nullptr);

    if (!ok)
    {
        error("Placement failed!\n");
        return;
    }

    done();
}
