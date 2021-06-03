#include "cellplacer.h"
#include "netlist/instance.h"
#include "common/dbtypes.h"
#include "common/logging.h"

using namespace LunaCore;

void SimpleCellPlacer::place(ChipDB::Netlist *nl, const ChipDB::Rect64 &regionRect, const int64_t rowHeight)
{
    ChipDB::Coord64 curPos = regionRect.m_ll;
    for(auto ins : nl->m_instances)
    {
        auto cellSize = ins->cellSize();

        // is there space for the next cell in the current row?
        if (cellSize.m_x + curPos.m_x > regionRect.m_ur.m_x)
        {
            // no, got to next row
            curPos.m_x = regionRect.m_ll.m_x;
            curPos.m_y += rowHeight;
        }

        ins->m_pos = curPos;
        curPos.m_x += cellSize.m_x;
    }
    
    if (curPos.m_y >= regionRect.m_ur.m_y)
    {
        doLog(LOG_ERROR, "SimpleCellPlacer: not enough room in region for all instances\n");
    }
}
