// SPDX-FileCopyrightText: 2021-2023 Niels Moseley <asicsforthemasses@gmail.com>
//
// SPDX-License-Identifier: GPL-3.0-only

#include <limits>
#include <fstream>
#include "grid.h"
#include "export/ppm/ppmwriter.h"

using namespace LunaCore;

GlobalRouter::Grid::Grid(const GCellCoordType width, const GCellCoordType height,
    const ChipDB::Size64 &cellSize)
{
    assert(width >= 0);
    assert(height >= 0);
    assert(cellSize.m_x >= 0);
    assert(cellSize.m_y >= 0);

    m_width  = width;
    m_height = height;
    m_cellSize = cellSize;
    m_grid.resize(width*height);
    clearGrid();
}

GlobalRouter::GCellCoord GlobalRouter::Grid::toGridCoord(const ChipDB::Coord64 &p) const noexcept
{
    return {p.m_x / m_cellSize.m_x, p.m_y / m_cellSize.m_y};
}

bool GlobalRouter::Grid::isValidGridCoord(const GCellCoord &loc) const noexcept
{
    return (loc.m_x >= 0) && (loc.m_x < m_width) && (loc.m_y >= 0) && (loc.m_y < m_height);
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

void GlobalRouter::Grid::clearReachedAndResetCost()
{
    for(auto &cell : m_grid)
    {
        cell.clearReached();
        cell.m_cost = std::numeric_limits<decltype(cell.m_cost)>::max();
    }
}

PPM::Bitmap GlobalRouter::Grid::generateCapacityBitmap() const noexcept
{
    constexpr PPM::RGB uncongestedColor{0,255,0,0};
    constexpr PPM::RGB congestedColor{255,0,0,0};

    PPM::Bitmap bm;
    bm.m_width = width();
    bm.m_height = height();
    bm.m_data.resize(width()*height());

    for(int y=0; y<height(); ++y)
    {
        for(int x=0; x<width(); ++x)
        {
            PPM::RGB pixel{0,0,0,0};

            auto capacity = at(x,y).m_capacity;
            if (capacity > m_maxCapacity) capacity = m_maxCapacity;

            pixel = interpolate(uncongestedColor, congestedColor, capacity/static_cast<float>(m_maxCapacity));

            if ((x == 0) || (y == 0) || (x == width()-1) || (y == height()-1))
            {
                pixel = {255,255,255,0};
            }

            bm.m_data.at(width()*y + x) = pixel;
        }
    }

    return std::move(bm);
}


PPM::Bitmap GlobalRouter::Grid::generateBitmap() const noexcept
{
    constexpr PPM::RGB netColor{0,255,0,0};
    constexpr PPM::RGB congestedColor{255,0,0,0};

    constexpr PPM::RGB terminalColor{255,0,255,0};
    constexpr PPM::RGB blockColor{255,0,0,0};

    PPM::Bitmap bm;
    bm.m_width = width();
    bm.m_height = height();
    bm.m_data.resize(width()*height());

    for(int y=0; y<height(); ++y)
    {
        for(int x=0; x<width(); ++x)
        {
            PPM::RGB pixel{0,0,0,0};

            auto capacity = at(x,y).m_capacity;
            if (capacity > m_maxCapacity) capacity = m_maxCapacity;

            if (at(x,y).isMarked()) pixel = interpolate(netColor, congestedColor, capacity/static_cast<float>(m_maxCapacity));
            if (at(x,y).isTarget()) pixel = interpolate(netColor, congestedColor, capacity/static_cast<float>(m_maxCapacity));
            if (at(x,y).isSource()) pixel = interpolate(netColor, congestedColor, capacity/static_cast<float>(m_maxCapacity));

            if ((x == 0) || (y == 0) || (x == width()-1) || (y == height()-1))
            {
                pixel = {255,255,255,0};
            }

            if (at(x,y).isBlocked())
            {
                pixel = blockColor;
            }

            bm.m_data.at(width()*y + x) = pixel;
        }
    }

    return std::move(bm);
}

#if 0
bool GlobalRouter::Grid::exportToPPM(const std::string &filename) const
{
    std::ofstream ofile(filename);
    if (!ofile.good()) return false;
    if (!ofile.is_open()) return false;


#if 0
    for(auto const& net : nets)
    {
        for(auto const& terminal : net.m_points)
        {        
            if (terminal.m_y >= 1) bitmap.at(grid.width()*(terminal.m_y-1) + terminal.m_x) = terminalColor;
            bitmap.at(grid.width()*(terminal.m_y+1) + terminal.m_x) = terminalColor;
            bitmap.at(grid.width()*terminal.m_y + terminal.m_x + 1) = terminalColor;
            if (terminal.m_x >= 1) bitmap.at(grid.width()*terminal.m_y + terminal.m_x - 1) = terminalColor;
        }
    }
#endif

    PPM::write(ofile, bitmap, width());

    return true;
}
#endif

void GlobalRouter::Grid::clearAllFlagsAndResetCost()
{   
    for(auto &cell : m_grid)
    {
        cell.resetFlags();
        cell.m_cost = std::numeric_limits<decltype(cell.m_cost)>::max();
    }
}

void GlobalRouter::Grid::clearGrid()
{
    for(auto &cell : m_grid)
    {
        cell.resetFlags();
        cell.m_capacity = 0;
        cell.m_cost = std::numeric_limits<decltype(cell.m_cost)>::max();
    }
}
