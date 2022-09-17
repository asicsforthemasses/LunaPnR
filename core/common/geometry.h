// SPDX-FileCopyrightText: 2021-2022 Niels Moseley <asicsforthemasses@gmail.com>
//
// SPDX-License-Identifier: GPL-3.0-only

#pragma once
#include <iostream>
#include <vector>
#include <variant>
#include <list>

#include "dbtypes.h"

namespace ChipDB
{
class Rectangle
{
public:
    Rectangle(const Rect64 &r) : m_rect(r) {};

    ChipDB::Rect64 m_rect;
};

class Polygon
{
public:
    Polygon(const std::vector<Coord64> &points) : m_points(points) {};

    std::vector<ChipDB::Coord64> m_points;
};

using GeometryObject  = std::variant<Rectangle, Polygon>;
using GeometryObjects = std::vector<GeometryObject>;

/** defined an interval between x1 and x2 */
struct Interval
{
    CoordType x1;
    CoordType x2;

    [[nodiscard]] constexpr bool isValid() const noexcept
    {
        return x1 <= x2;
    }

    /** returns true if two intervals overlap */
    [[nodiscard]] bool overlap(const Interval &v) const noexcept
    {
        if ((v.x1 < x1) && (v.x2 < x1)) return false;
        if ((v.x1 > x2) && (v.x2 > x2)) return false;
        return true;
    }    

    bool operator == (const Interval &rhs) const noexcept
    {
        return (x1 == rhs.x1) && (x2 == rhs.x2);
    }

    bool operator != (const Interval &rhs) const noexcept
    {
        return (x1 != rhs.x1) || (x2 != rhs.x2);
    }

    bool operator < (const Interval &rhs) const noexcept
    {
        return (x2 < rhs.x1);
    }

    bool operator > (const Interval &rhs) const noexcept
    {
        return (x1 > rhs.x2);
    }

    [[nodiscard]] Interval merge(const Interval &other) const noexcept;
    [[nodiscard]] Interval common(const Interval &other) const noexcept;
};

/** list of non-overlapping intervals sorted in the x direction, ascending */
class IntervalList
{
public:
    
    [[nodiscard]] auto size() const noexcept { return m_intervals.size(); }

    [[nodiscard]] auto begin()    {return m_intervals.begin(); };
    [[nodiscard]] auto end()      {return m_intervals.end(); };

    [[nodiscard]] auto begin() const  {return m_intervals.begin(); };
    [[nodiscard]] auto end() const    {return m_intervals.end(); };

    /** add an interval to the list. sorting and merging is performed automatically */
    bool addInterval(const Interval &v);
    
    /** remove all intervals from the list */
    void clear();

protected:

    // vector of sorted/ascending x1 intervals
    std::list<Interval> m_intervals;  
};

};

std::ostream& operator<<(std::ostream& os, const ChipDB::Interval& v);
