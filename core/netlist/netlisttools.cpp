// SPDX-FileCopyrightText: 2021-2022 Niels Moseley <asicsforthemasses@gmail.com>
//
// SPDX-License-Identifier: GPL-3.0-only


#include "netlisttools.h"
#include "common/logging.h"


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

    for(auto ins : netlist->m_instances)
    {
        if (ins.isValid() && 
            ((ins->m_placementInfo == ChipDB::PlacementInfo::PLACED) || (ins->m_placementInfo == ChipDB::PlacementInfo::PLACEDANDFIXED)))
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

    for(const auto netKeyObjPair : netlist->m_nets)
    {
        hist[netKeyObjPair->numberOfConnections()]++;
    }

    return hist;
}

bool LunaCore::NetlistTools::removeNetconInstances(ChipDB::Netlist &netlist)
{
    //FIXME: this needs re-writing!
    
#if 0    
    size_t netsRemoved = 0;
    size_t degenerateNets = 0;
    for (auto insKeyObjPair : netlist.m_instances)
    {
        if (insKeyObjPair->getArchetypeName() == "__NETCON")
        {
            auto net1Key = insKeyObjPair->getPin(0).m_netKey;
            auto net2Key = insKeyObjPair->getPin(1).m_netKey;

            if ((net1Key < 0) || (net2Key < 0))
            {
                // handle degenerate net: remove NETCON            
                degenerateNets++;
            }
            else
            {
                auto netToRemove = net1Key;
                auto netToAssign = net2Key;

                for(auto const &connection : *netlist.m_nets.at(netToRemove))
                {
                    auto insToBeUpdated = netlist.m_instances.at(connection.m_instanceKey);
                    if (insToBeUpdated != netConIns.key())
                    {
                        insToBeUpdated->connect(connection.m_pinIndex, netToAssign);
                    }
                }

                netToRemove->m_connections.clear();
                delete ins;
                ins = nullptr;
                netsRemoved++;
            }
        }
    }
    std::stringstream ss;
    ss << "Removed " << netsRemoved << " netcon nets (found " << degenerateNets << " degerate nets)\n";
    doLog(LogType::VERBOSE, ss);
#endif    
    return true;
}

double LunaCore::NetlistTools::calcHPWL(const ChipDB::Netlist &netlist)
{
    double hpwl = 0;
    for(auto const netPtr : netlist.m_nets)
    {
        auto iter = netPtr->begin();

        // skip if there are no connection on this net.
        if (iter == netPtr->end())
            continue;

        // get first cell
        auto center = netlist.m_instances.at(iter->m_instanceKey)->getCenter();
        ChipDB::Coord64 minCoord{center.m_x, center.m_y};
        ChipDB::Coord64 maxCoord{center.m_x, center.m_y};
        ++iter;

        // process next cells
        while(iter != netPtr->end())
        {
            center = netlist.m_instances.at(iter->m_instanceKey)->getCenter();
            minCoord.m_x = std::min(minCoord.m_x, center.m_x);
            minCoord.m_y = std::min(minCoord.m_y, center.m_y);
            maxCoord.m_x = std::max(maxCoord.m_x, center.m_x);
            maxCoord.m_y = std::max(maxCoord.m_y, center.m_y);            
            ++iter;
        }

        // calculate final hpwl from the net extents
        auto delta = maxCoord - minCoord;
        hpwl += static_cast<double>(delta.m_x) + static_cast<double>(delta.m_y);
    }

    return hpwl;
}

double LunaCore::NetlistTools::calcTotalCellArea(const ChipDB::Netlist &netlist)
{
    double um2 = 0.0;
    for(auto const ins : netlist.m_instances)
    {
        auto csize = ins->instanceSize();
        double sx = static_cast<double>(csize.m_x) / 1000.0;
        double sy = static_cast<double>(csize.m_y) / 1000.0;
        um2 += sx*sy;
    }

    return um2;
}
