#include <algorithm>
#include "cellplacer.h"
#include "netlist/instance.h"
#include "common/dbtypes.h"
#include "common/logging.h"

using namespace LunaCore;

void SimpleCellPlacer::place(ChipDB::Netlist *nl, const ChipDB::Rect64 &regionRect, const int64_t rowHeight)
{
    bool flip = false;
    ChipDB::Coord64 curPos = regionRect.m_ll;
    for(auto ins : nl->m_instances)
    {
        // do not place pins
        if (ins->m_insType == ChipDB::InstanceBase::INS_PIN)
            continue;
            
        auto cellSize = ins->instanceSize();

        // is there space for the next cell in the current row?
        if ((cellSize.m_x + curPos.m_x) > regionRect.m_ur.m_x)
        {
            // no, got to next row
            curPos.m_x = regionRect.m_ll.m_x;
            curPos.m_y += rowHeight;

            flip = !flip;
        }

        ins->m_pos = curPos;
        curPos.m_x += cellSize.m_x;

        if (flip)
            ins->m_orientation = ChipDB::Orientation::MX;
        else
            ins->m_orientation = ChipDB::Orientation::R0;
    }

    if (curPos.m_y >= regionRect.m_ur.m_y)
    {
        std::stringstream ss;
        ss << "SimpleCellPlacer: not enough room in region for all instances\n";
        ss << "  need at least " << (curPos.m_y - regionRect.m_ur.m_y) << " nm more height\n";
        doLog(LOG_ERROR, ss);
    }
}

int64_t HPWLCalculator::calc(ChipDB::Netlist *nl)
{
    int64_t hpwl = 0;
    for(auto const net : nl->m_nets)
    {
        auto iter = net->m_connections.begin();

        // skip if there are no connection on this net.
        if (iter == net->m_connections.end())
            continue;

        // get first cell
        auto center = iter->m_instance->getCenter();
        ChipDB::Coord64 minCoord{center.m_x, center.m_y};
        ChipDB::Coord64 maxCoord{center.m_x, center.m_y};
        ++iter;

        // process next cells
        while(iter != net->m_connections.end())
        {
            center = iter->m_instance->getCenter();
            minCoord.m_x = std::min(minCoord.m_x, center.m_x);
            minCoord.m_y = std::min(minCoord.m_y, center.m_y);
            maxCoord.m_x = std::max(maxCoord.m_x, center.m_x);
            maxCoord.m_y = std::max(maxCoord.m_y, center.m_y);            
            ++iter;
        }

        // calculate final hpwl from the net extents
        auto delta = maxCoord - minCoord;
        hpwl += delta.m_x + delta.m_y;
    }

    return hpwl;
}

double CellAreaCalculator::calc(ChipDB::Netlist *nl)
{
    double um2 = 0.0;
    for(auto const ins : nl->m_instances)
    {
        auto csize = ins->instanceSize();
        double sx = static_cast<double>(csize.m_x) / 1000.0;
        double sy = static_cast<double>(csize.m_y) / 1000.0;
        um2 += sx*sy;
    }

    return um2;
}
