#pragma once

#include <cstdint>
#include <vector>
#include <iostream>
#include <memory>

#include "common/dbtypes.h"
#include "netlist/netlist.h"
#include "floorplan/region.h"

namespace LunaCore
{

/** Instance/cell density bitmap */
class DensityBitmap
{
public:
    using PixelType = float;

    DensityBitmap(ssize_t width, size_t height) : m_dummy(0)
    {
        setSizeAndClear(width, height);
    }

    void setSizeAndClear(ssize_t width, size_t height)
    {
        m_bitmap.clear();

        if ((width >= 0) && (height >= 0))
        {
            m_width  = width;
            m_height = height;

            const PixelType initialValue = 0;
            m_bitmap.resize(width * height, initialValue);
        }
        else
        {
            m_width = 0;
            m_height = 0;
        }
    }


    /** get reference to a pixel. Does not throw when out of bounds but returns a dummy element containing zero. */
    PixelType& at(const ChipDB::Coord64 &pos) noexcept
    {
        if ((pos.m_x < 0) || (pos.m_x >= m_width))
        {
            m_dummy = 0;
            return m_dummy;
        }

        if ((pos.m_y < 0) || (pos.m_y >= m_height))
        {
            m_dummy = 0;
            return m_dummy;
        }

        return m_bitmap[pos.m_x + pos.m_y * m_width];
    }

    PixelType& at(int64_t x, int64_t y) noexcept
    {
        if ((x < 0) || (x >= m_width))
        {
            m_dummy = 0;
            return m_dummy;
        }

        if ((y < 0) || (y >= m_height))
        {
            m_dummy = 0;
            return m_dummy;
        }

        return m_bitmap[x + y * m_width];
    }    

    void writeToPGM(std::ostream &os);

protected:
    PixelType               m_dummy;
    ssize_t                 m_width;
    ssize_t                 m_height;
    std::vector<PixelType>  m_bitmap;
};

//FIXME: this really should be called with a region of interest
//       and not a netlist
//       except that a region does not (yet) know
//       which instances are present.

std::shared_ptr<DensityBitmap> createDensityBitmap(const ChipDB::Netlist *netlist, const ChipDB::Region *region,
    int64_t bitmapCellWidth /* nm */, int64_t bitmapCellHeight /* nm */);

};