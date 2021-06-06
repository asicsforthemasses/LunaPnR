#include <algorithm>
#include "fmpart.h"

#include "netlist/instance.h"

using namespace LunaCore;

int64_t FMPart::distanceToPartition(const Partition &part, const ChipDB::Coord64 &pos)
{
    auto dx = std::max(part.m_region.m_ll.m_x - pos.m_x, int64_t(0));
         dx = std::max(dx, pos.m_x - part.m_region.m_ur.m_x);

    auto dy = std::max(part.m_region.m_ll.m_y - pos.m_y, int64_t(0));
         dy = std::max(dy, pos.m_y - part.m_region.m_ur.m_y);

    return dx+dy;
}

bool FMPart::init(ChipDB::Netlist *nl)
{
    if (nl == nullptr)
        return false;

    m_nodes.resize(nl->m_instances.size());
    m_nets.resize(nl->m_nets.size());

    // use the flags of the instance and 
    // nets to number each entity uniquely

    ssize_t id = 0;
    for(auto ins : nl->m_instances)
    {
        ins->m_flags = id;

        /* use cell width as the weight, as the height of each cell is the same */
        m_nodes[id].m_weight = ins->instanceSize().m_x;

        // check if the instance has a fixed position.
        // if so, assign the instance/node to the closest
        // partition

        if (ins->m_placementInfo == ChipDB::PlacementInfo::PLACEMENT_PLACEDANDFIXED)
        {
            if (distanceToPartition(m_part1, ins->m_pos) < distanceToPartition(m_part2, ins->m_pos))
            {
                m_nodes[id].m_partitionId = 1;
            }
            else
            {
                m_nodes[id].m_partitionId = 2;
            }
            m_nodes[id].m_locked = true;
        }
        else
        {
            // randomly assign a partition
            // FIXME: do some area / fill balancing
            m_nodes[id].m_locked = false;
            if (std::rand() > (RAND_MAX/2))
            {
                m_nodes[id].m_partitionId = 2;
            }
            else
            {
                m_nodes[id].m_partitionId = 1;
            }
        }

        id++;
    }

    id = 0;
    for(auto net : nl->m_nets)
    {
        net->m_flags = id;
        m_nets[id].m_weight = 1;    // all nets are the same weight
        id++;
    }

    // set the connected nets of each node
    // and add the node to each net
    for(auto ins : nl->m_instances)
    {
        for(ssize_t pinIndex=0; pinIndex < ins->getNumberOfPins(); ++pinIndex)
        {
            auto net = ins->getConnectedNet(pinIndex);
            if (net != nullptr)
            {
                m_nodes[ins->m_flags].m_nets.push_back(net->m_flags);
                m_nets[net->m_flags].m_nodes.push_back(ins->m_flags);
            }
        }
    }

    return true;
}

