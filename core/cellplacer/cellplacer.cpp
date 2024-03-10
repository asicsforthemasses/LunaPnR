// SPDX-FileCopyrightText: 2021-2024 Niels Moseley <asicsforthemasses@gmail.com>
//
// SPDX-License-Identifier: GPL-3.0-only

#include <algorithm>
#include "cellplacer.h"
#include "database/database.h"
#include "common/logging.h"

using namespace LunaCore;

void SimpleCellPlacer::place(ChipDB::Netlist *nl, const ChipDB::Rect64 &regionRect, const int64_t rowHeight)
{
    bool flip = false;
    ChipDB::Coord64 curPos = regionRect.m_ll;
    for(auto ins : nl->m_instances)
    {
        // do not place pins
        if (ins->isPin())
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
        Logging::doLog(Logging::LogType::ERROR, ss);
    }
}

