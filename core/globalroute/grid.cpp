// SPDX-FileCopyrightText: 2021-2022 Niels Moseley <asicsforthemasses@gmail.com>
//
// SPDX-License-Identifier: GPL-3.0-only

#include "grid.h"

using namespace LunaCore;

GlobalRouter::GCellCoord GlobalRouter::Grid::toGridCoord(const ChipDB::Coord64 &p) const noexcept
{
    return {p.m_x / m_cellSize.m_x, p.m_y / m_cellSize.m_y};
}

const GlobalRouter::GCell& GlobalRouter::Grid::at(const GCellCoordType x, const GCellCoordType y) const
{
    if ((x >= m_width) || (x < 0))
    {
        m_invalidCell.setInvalid();
        return m_invalidCell;
    }

    return m_grid.at(y*m_width + x);
}

GlobalRouter::GCell& GlobalRouter::Grid::at(const GCellCoordType x, const GCellCoordType y)
{
    if ((x >= m_width) || (x < 0))
    {
        m_invalidCell.setInvalid();
        return m_invalidCell;
    }

    return m_grid.at(y*m_width + x);
}

const GlobalRouter::GCell& GlobalRouter::Grid::at(const GCellCoord &p) const
{
    if ((p.m_x >= m_width) || (p.m_x < 0))
    {
        m_invalidCell.setInvalid();
        return m_invalidCell;
    }

    return m_grid.at(p.m_y*m_width + p.m_x);
}

GlobalRouter::GCell& GlobalRouter::Grid::at(const GCellCoord &p)
{
    if ((p.m_x >= m_width) || (p.m_x < 0))
    {
        m_invalidCell.setInvalid();
        return m_invalidCell;
    }

    return m_grid.at(p.m_y*m_width + p.m_x);
}
