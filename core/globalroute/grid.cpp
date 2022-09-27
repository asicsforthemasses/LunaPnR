// SPDX-FileCopyrightText: 2021-2022 Niels Moseley <asicsforthemasses@gmail.com>
//
// SPDX-License-Identifier: GPL-3.0-only

#include <limits>
#include <fstream>
#include "grid.h"

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
    clear();
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

void GlobalRouter::Grid::clear()
{
    for(auto &cell : m_grid)
    {
        cell.clearReached();
        cell.m_cost = std::numeric_limits<decltype(cell.m_cost)>::max();
    }
}

bool GlobalRouter::Grid::exportToPGM(const std::string &filename) const
{
    std::ofstream ofile(filename);
    if (!ofile.good()) return false;
    if (!ofile.is_open()) return false;

    int maxColorValue = 255;
    ofile << "P6\n" << width() << " " << height() << "\n" << maxColorValue << "\n";

    constexpr RGB netColor{0,255,0,0};
    constexpr RGB congestedColor{255,0,0,0};

    constexpr RGB terminalColor{255,0,255,0};
    constexpr RGB blockColor{255,0,0,0};

    std::vector<RGB> bitmap(width()*height());

    for(int y=0; y<height(); ++y)
    {
        for(int x=0; x<width(); ++x)
        {
            RGB pixel{0,0,0,0};

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

            bitmap.at(width()*y + x) = pixel;
        }
    }

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

    for(int y=0; y<height(); ++y)
    {
        auto h_offset = (height() - y - 1) * width();
        for(int x=0; x<width(); ++x)
        {
            ofile.write(reinterpret_cast<const char*>(&bitmap.at(h_offset + x)), 3);
        }
    }    

    return true;
}
