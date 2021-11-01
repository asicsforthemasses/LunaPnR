#include "netlisttools.h"


bool LunaCore::NetlistTools::writePlacementFile(std::ostream &os, const ChipDB::Netlist *netlist)
{
    if (!os.good())
    {
        return false;
    }

    if (netlist == nullptr)
    {
        return false;
    }

    for(auto const* ins : netlist->m_instances)
    {
        if ((ins->m_placementInfo == ChipDB::PlacementInfo::PLACED) || (ins->m_placementInfo == ChipDB::PlacementInfo::PLACEDANDFIXED))
        {
            auto insSize = ins->instanceSize();
            os << ins->m_pos.m_x << " " << ins->m_pos.m_y << " " << insSize.m_x << " " << insSize.m_y << "\n";
        }
    }

    return true;
}


LunaCore::NetlistTools::NetlistHistogram LunaCore::NetlistTools::calcNetlistHistogram(const ChipDB::Netlist *netlist)
{
    NetlistHistogram hist;

    if (netlist == nullptr)
    {
        return hist;
    }

    for(const auto net : netlist->m_nets)
    {
        hist[net->m_connections.size()]++;
    }

    return hist;
}
