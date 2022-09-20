#include <algorithm>
#include "geometry.h"

using namespace ChipDB;

Interval Interval::merge(const Interval &other) const noexcept
{
    return {std::min(x1, other.x1), std::max(x2, other.x2)};
}

Interval Interval::common(const Interval &other) const noexcept
{
    return {std::max(x1, other.x1), std::min(x2, other.x2)};
}


bool IntervalList::addInterval(const Interval &v1)
{
    if (!v1.isValid()) return false;

    if (m_intervals.size() == 0)
    {
        m_intervals.push_back(v1);
        return true;
    }

    // first search for an interval that includes v.x1 or v.x2, that covers s1 and s3
    auto lower = m_intervals.end();

    auto iter = m_intervals.begin();
    while(iter != m_intervals.end())
    {
        if (iter->x2 < v1.x1) lower = iter;
        if (iter->overlap(v1)) break;
        iter++;
    }

    if (iter == m_intervals.end())
    {
        if (lower == m_intervals.end())
        {
            m_intervals.push_front(v1);
        }
        else
        {
            m_intervals.insert(++lower, v1);
        }
    }
    else
    {
        // overlap -> merge intervals
        //
        // create a merged interval and keep on
        // merging until there are no more overlaps
        
        auto v2 = *iter;
        auto merged = v1.merge(v2);
        iter = m_intervals.erase(iter);
        while ((iter != m_intervals.end()) && (merged.overlap(*iter)))
        {
            v2 = *iter;
            merged  = merged.merge(v2);
            iter = m_intervals.erase(iter);
        }
        m_intervals.insert(iter, merged);
    }

    return false;
}

void IntervalList::clear()
{
    m_intervals.clear();
}

bool IntervalList::contains(const CoordType &p) const noexcept
{
    for(auto const& interval : m_intervals)
    {
        if (interval.contains(p)) return true;
    }
    return false;
}

std::ostream& operator<<(std::ostream& os, const ChipDB::Interval& v)
{
    os << v.x1 << ".." << v.x2;
    return os;    
}

std::ostream& operator<<(std::ostream& os, const ChipDB::IntervalList& v)
{
    std::size_t count = v.size();
    for(auto const element : v)
    {
        os << element;
        if (count != 1)
        {
            os << " ";
        }
        count--;
    }
    return os;    
}

void ChipDB::findPinLocations(const GeometryObjects &objs,
    const ChipDB::Size64 &cellSize,
    const ChipDB::Coord64 &routingPitch,
    const ChipDB::Coord64 &routingOffset)
{
    // find horizontal intersections
    CoordType ypos = routingOffset.m_y;
    while(ypos < cellSize.m_y)
    {
        IntervalList xIntersections;
        for(auto const &obj : objs)
        {
            if (std::holds_alternative<ChipDB::Polygon>(obj))
            {
                // polygon
            }
            else
            {
                auto const& rect = std::get<ChipDB::Rectangle>(obj);
                // use min/max in case the rect doesn't adhere
                // to the conventions.
                auto maxy = std::max(rect.top(), rect.bottom());
                auto miny = std::min(rect.top(), rect.bottom());
                auto maxx = std::max(rect.left(), rect.right());
                auto minx = std::min(rect.left(), rect.right());
                if ((ypos >= miny) && (ypos <= maxy))
                {
                    xIntersections.addInterval({minx,maxx});
                }
            }
        }
        std::cout << "x Intersections at y = " << ypos << " :" << xIntersections << "\n";
    }
}
