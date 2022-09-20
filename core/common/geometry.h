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

    constexpr auto left() const noexcept {return m_rect.left(); }
    constexpr auto right() const noexcept {return m_rect.right(); }
    constexpr auto bottom() const noexcept {return m_rect.bottom(); }
    constexpr auto top() const noexcept {return m_rect.top(); }

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

    /** returns true if x1 <= x2 */
    [[nodiscard]] constexpr bool isValid() const noexcept
    {
        return x1 <= x2;
    }

    /** returns true if two (closed) intervals overlap */
    [[nodiscard]] bool overlap(const Interval &v) const noexcept
    {
        if ((v.x1 < x1) && (v.x2 < x1)) return false;
        if ((v.x1 > x2) && (v.x2 > x2)) return false;
        return true;
    }    

    /** return true if coordinate p is within the (closed) interval */
    [[nodiscard]] constexpr bool contains(const CoordType &p) const noexcept
    {
        return (x1 <= p) && (x2 >= p);
    }

    /** returns true if two intervals have the same limits */
    bool operator == (const Interval &rhs) const noexcept
    {
        return (x1 == rhs.x1) && (x2 == rhs.x2);
    }

    /** returns true if two intervals do not have a common limit */
    bool operator != (const Interval &rhs) const noexcept
    {
        return (x1 != rhs.x1) || (x2 != rhs.x2);
    }

    /** return true if rhs if full to the left of this interval */
    bool operator < (const Interval &rhs) const noexcept
    {
        return (x2 < rhs.x1);
    }
    
    /** return true if rhs if full to the right of this interval */
    bool operator > (const Interval &rhs) const noexcept
    {
        return (x1 > rhs.x2);
    }

    /** return the binary or of the two intervals */
    [[nodiscard]] Interval merge(const Interval &other) const noexcept;

    /** return the binary and of the two intervals */
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

    /** returns true if coordinate p is within the (closed) interval */
    [[nodiscard]] bool contains(const CoordType &p) const noexcept;

protected:

    // vector of sorted/ascending x1 intervals
    std::list<Interval> m_intervals;  
};

/** find the valid pin locations of a cell pin, given the rectangles/polygons describing 
 *  the cell pin.
*/
void findPinLocations(const GeometryObjects &objs, 
    const ChipDB::Size64 &cellSize,
    const ChipDB::Coord64 &routingPitch,
    const ChipDB::Coord64 &routingOffset);

};  // namespace

std::ostream& operator<<(std::ostream& os, const ChipDB::Interval& v);
std::ostream& operator<<(std::ostream& os, const ChipDB::IntervalList& v);

