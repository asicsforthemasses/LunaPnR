#include <unordered_map>
#include "common/logging.h"
#include "cellplacer.h"
#include "qlaplacer.h"

using namespace LunaCore::QLAPlacer;


bool LunaCore::QLAPlacer::place(const ChipDB::Rect64 &regionRect, const ChipDB::Module *mod)
{
    double area = 0.0f;

    doLog(LOG_INFO, "Placing module %s in rectangle (%d,%d)-(%d,%d).\n",
        mod->m_name.c_str(),
        regionRect.left(), regionRect.bottom(), 
        regionRect.right(), regionRect.top());

    // check if pins have been fixed
    // and total the area
    for(auto ins : mod->m_netlist->m_instances)
    {
        if (ins->m_insType == ChipDB::InstanceType::PIN)
        {
            if (ins->m_placementInfo != ChipDB::PlacementInfo::PLACEDANDFIXED)
            {
                std::stringstream ss;
                ss << "Not all pins have been placed and fixed - for example: " << ins->m_name << "\n";
                doLog(LOG_ERROR, ss);
                return false;
            }
        }

        area += ins->instanceSize().m_x * ins->instanceSize().m_y;
    }

    const auto regionArea = regionRect.width() * regionRect.height();

    if (regionArea < area)
    {
        std::stringstream ss;
        ss << "The region area (" << regionArea << ") is smaller than the total instance area (" << area << ")\n";
        doLog(LOG_ERROR, ss);
        return false;
    }

    doLog(LOG_INFO, "Utilization = %3.1f percent\n", 100.0* area / static_cast<double>(regionArea));

    auto &modNetlist = *mod->m_netlist.get();
    auto netlist = Private::createPlacerNetlist(modNetlist);
    
    Private::doInitialPlacement(regionRect, netlist);    
    Private::updatePositions(netlist, modNetlist);
    for(uint32_t i=0; i<20; i++)
    {
        Private::doQuadraticB2B(netlist);
        Private::updatePositions(netlist, modNetlist);
        
        doLog(LOG_INFO,"Iteration %d HPWL %f\n", i, Private::calcHPWL(netlist));
    }
    

    doLog(LOG_INFO, "Placement done.\n");

    return true;
}
