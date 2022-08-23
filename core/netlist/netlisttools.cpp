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
