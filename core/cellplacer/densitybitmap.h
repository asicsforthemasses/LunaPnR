// SPDX-FileCopyrightText: 2021-2022,2023 Niels Moseley <asicsforthemasses@gmail.com>
//
// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <cstdint>
#include <vector>
#include <iostream>
#include <memory>

#include "common/dbtypes.h"
#include "netlist/netlist.h"
#include "floorplan/region.h"

namespace LunaCore::QPlacer
{

class TerminationInstance : public ChipDB::Instance
{
public:
    /** get area in um² */
    double getArea() const noexcept override
    {
        return 0;
    }

    /** return the underlying cell/module name */
    std::string getArchetypeName() const override
    {
        return "TerminationInstance";
    }
    
    /** return the size of the instance in nm */
    const ChipDB::Coord64 instanceSize() const override
    {
        return {0,0};
    }

    /** return the center position of the instance */
    ChipDB::Coord64 getCenter() const override
    {
        return m_pos;
    }

    size_t getNumberOfPins() const override
    {
        return 0;
    }

    Pin getPin(const std::string &pinName) const override
    {
        return Pin();
    }

    Pin getPin(ChipDB::PinObjectKey key) const override
    {
        return Pin();
    }

    bool setPinNet(ChipDB::PinObjectKey key1, ChipDB::NetObjectKey key2) override
    {
        return false;
    }

};

/** Instance/cell density bitmap */
template<class PixelType>
class Bitmap
{
public:
    Bitmap(ssize_t width, size_t height) : m_dummy(0)
    {
        setSizeAndClear(width, height);
    }

    ssize_t width() const noexcept
    {
        return m_width;
    }

    ssize_t height() const noexcept
    {
        return m_height;
    }

    size_t size() const noexcept
    {
        return m_bitmap.size();
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

    const PixelType& at(int64_t x, int64_t y) const noexcept
    {
        if ((x < 0) || (x >= m_width))
        {
            return m_constDummy;
        }

        if ((y < 0) || (y >= m_height))
        {
            return m_constDummy;
        }

        return m_bitmap[x + y * m_width];
    }        


protected:
    static constexpr PixelType        m_constDummy = 0;
    PixelType               m_dummy;
    ssize_t                 m_width;
    ssize_t                 m_height;
    std::vector<PixelType>  m_bitmap;
};

struct Velocity
{
    Velocity() = default;
    constexpr Velocity(float v) : m_dx(v), m_dy(v) {};
    constexpr Velocity(float x, float y) : m_dx(x), m_dy(y) {};

    float m_dx;
    float m_dy;

    [[nodiscard]] Velocity operator+(const Velocity &rhs) const noexcept
    {
        return Velocity{m_dx+rhs.m_dx, m_dy+rhs.m_dy};
    }

    [[nodiscard]] Velocity operator-(const Velocity &rhs) const noexcept
    {
        return Velocity{m_dx-rhs.m_dx, m_dy-rhs.m_dy};
    }

    [[nodiscard]] Velocity operator*(const float &rhs) const noexcept
    {
        return Velocity{m_dx*rhs, m_dy*rhs};
    }     
};

Velocity operator*(const float &lhs, const Velocity &rhs);
bool operator==(const Velocity &lhs, const Velocity &rhs);

using DensityBitmap  = Bitmap<float>;
using VelocityBitmap = Bitmap<Velocity>;

//FIXME: this really should be called with a region of interest
//       and not a netlist
//       except that a region does not (yet) know
//       which instances are present.

DensityBitmap* createDensityBitmap(const ChipDB::Netlist *netlist, const ChipDB::Region *region,
    const int64_t bitmapCellWidth /* nm */, 
    const int64_t bitmapCellHeight /* nm */);

void calcVelocityBitmap(const DensityBitmap *bm, VelocityBitmap *vm);

void updateMovableInstances(ChipDB::Netlist *netlist, const ChipDB::Region *region, 
    VelocityBitmap *vm, 
    const int64_t bitmapCellWidth, 
    const int64_t bitmapCellHeight);

void setMinimalDensities(DensityBitmap *bm, const float maxDensity);

float updateDensityBitmap(DensityBitmap *bm);

void writeToPGM(std::ostream &os, const DensityBitmap *bitmap);

Velocity interpolateVelocity(const VelocityBitmap *vbitmap, 
    const int64_t bitmapCellWidth, const int64_t bitmapCellHeight,
    const ChipDB::Coord64 &instanceCenter);

};