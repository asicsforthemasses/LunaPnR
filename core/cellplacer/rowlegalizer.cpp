#include <vector>
#include <algorithm>
#include "common/logging.h"
#include "rowlegalizer.h"

using namespace LunaCore;

bool RowLegalizer::legalize(const ChipDB::Design &design, ChipDB::Module &mod, ChipDB::Region &region)
{
    // check that all the instances in the module have been placed
    if (!checkAllInstancesPlaced(mod))
    {
        doLog(LOG_ERROR, "Legalization requires that all instanced have been placed\n");
        return false;
    }

    auto &instances = mod.m_netlist->m_instances;

    // sort instances by x-coordinate
    std::vector<ChipDB::InstanceBase*> sortedIns(instances.size());
    for(uint32_t idx=0; idx<instances.size(); idx++)
    {
        sortedIns.at(idx) = instances.at(idx);
    }

    std::sort(sortedIns.begin(), sortedIns.end(), [&sortedIns](const auto ins1, const auto ins2)
        {
            if (ins1->m_pos.m_x < ins2->m_pos.m_x)
            {
                return true;
            }
            return false;
        }
    );

    std::vector<RowInfo> rowInfos(region.m_rows.size());
    for(size_t rowIdx=0; rowIdx<region.m_rows.size(); rowIdx++)
    {
        rowInfos.at(rowIdx).m_rect = region.m_rows.at(rowIdx).m_rect;
    }

    for(auto ins : sortedIns)
    {
        // skip fixed cells/instances
        if (ins->m_placementInfo == ChipDB::PlacementInfo::PLACEDANDFIXED)
        {
            continue;
        }

        double bestCost = std::numeric_limits<double>::max();
        ssize_t bestRow = -1;

        ssize_t rowIdx = 0;
        for(auto &rowInfo : rowInfos)
        {
            double cost = insertInstance(ins, rowInfo, InsertMode::Trial);
            if (cost < bestCost)
            {
                bestCost = cost;
                bestRow = rowIdx;
            }
            rowIdx++;
        }

        if (bestRow >=0)
        {
            insertInstance(ins, rowInfos.at(bestRow), InsertMode::Place);
        }
        else
        {
            doLog(LOG_ERROR, "Cannot find a row to place instance %s\n", ins->m_name.c_str());
            return false;
        }
    }

    return true;
}

bool RowLegalizer::checkAllInstancesPlaced(const ChipDB::Module &module)
{
    for(auto ins : module.m_netlist->m_instances)
    {
        if ((ins->m_placementInfo != ChipDB::PlacementInfo::PLACED) && (ins->m_placementInfo != ChipDB::PlacementInfo::PLACEDANDFIXED))
        {
            return false;
        }
    }
    return true;
}

double RowLegalizer::insertInstance(ChipDB::InstanceBase *ins, RowInfo &rowInfo, InsertMode mode)
{
    double cost = std::numeric_limits<double>::max();
    auto occupiedRowArea = ins->getArea() + rowInfo.getOccupiedArea();
    if (occupiedRowArea > rowInfo.getAvailableArea())
    {
        return cost;
    }
    
    auto oldRow = rowInfo;

    auto insPos   = ins->m_pos;
    const auto insWidth = ins->instanceSize().m_x;
    const auto rowY     = rowInfo.m_rect.getLL().m_y;
    
    if (!rowInfo.hasOverlap(insPos.m_x, insWidth))
    {
        rowInfo.createCluster(insPos.m_x, insWidth);
        cost = std::abs(ins->m_pos.m_y - rowY);
    }
    else
    {
        cost = rowInfo.addAndCollapseCluster(insPos.m_x, insWidth);
    }

    if (mode == InsertMode::Trial)
    {
        rowInfo = oldRow;
    }

    return cost;
}



// ================================================================================
//   RowInfo
// ================================================================================

ssize_t RowLegalizer::RowInfo::hasOverlap(ChipDB::CoordType xpos, ChipDB::CoordType width) const
{
    //note: clusters are sorted from left to right
    
    ssize_t clusterIdx = 0;
    while(clusterIdx < m_clusters.size())
    {
        auto &cluster = m_clusters.at(clusterIdx);
        const auto clusterEnd = cluster.m_start + cluster.m_width;

        auto left  = std::max(cluster.m_start, xpos);
        auto right = std::min(clusterEnd, xpos+width);

        if (left <= right)
        {
            // overlap!
            return clusterIdx;
        }

        if (cluster.m_start > xpos)
        {
            // there can be no more clusters
            // overlapping the given interval
            return -1;
        }

        clusterIdx++;
    }

    return -1; // no overlap
}

void RowLegalizer::RowInfo::createCluster(ChipDB::CoordType xpos, ChipDB::CoordType width)
{
    // find the insertion spot
    auto iter = m_clusters.begin();    
    while((iter != m_clusters.end()) && ((iter->m_start+iter->m_width) < xpos))
    {
        iter++;
    }

    m_clusters.insert(iter, 1, Cluster{xpos, width});    
}

double RowLegalizer::RowInfo::addAndCollapseCluster(ChipDB::CoordType xpos, ChipDB::CoordType width)
{
    return 0.0;
}
