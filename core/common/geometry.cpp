#include <algorithm>
#include "geometry.h"

using namespace ChipDB;

std::optional<ChipDB::Rectangle> ChipDB::Rectangle::intersect(const Rectangle &r) const noexcept
{
    auto const& ll1 = m_rect.m_ll;
    auto const& ll2 = r.m_rect.m_ll;
    auto const& ur1 = m_rect.m_ur;
    auto const& ur2 = r.m_rect.m_ur;

    // check if the rectangles intersect
    if ((ll1.m_x <= ur2.m_x) && (ur1.m_x >= ll2.m_x) &&
        (ll1.m_y <= ur2.m_y) && (ur1.m_y >= ll2.m_y))
    {
        auto ll_x = std::max(ll1.m_x, ll2.m_x);
        auto ll_y = std::max(ll1.m_y, ll2.m_y);
        auto ur_x = std::min(ur1.m_x, ur2.m_x);
        auto ur_y = std::min(ur1.m_y, ur2.m_y);

        // avoid returning rectangles that have area=0
        if ((ll_x == ur_x) || (ll_y == ur_y))
        {
            return std::nullopt;
        }

        return ChipDB::Rect64{{ll_x, ll_y}, {ur_x, ur_y}};
    }
    else
    {
        // no intersection
        return std::nullopt;
    }
}


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
    if (v.isValid())
    {
        os << v.x1 << ".." << v.x2;
    }
    else
    {
        os << "(empty)";
    }
    return os;    
}

std::ostream& operator<<(std::ostream& os, const ChipDB::IntervalList& v)
{
    std::size_t count = v.size();
    if (count == 0)
    {
        std::cout << "(empty)";
    }

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
    const ChipDB::CoordType routingWidth,
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
                
                // intersect top of wire
                auto wTop = ypos + routingWidth/2;
                auto wBot = ypos - routingWidth/2;

                Interval wireTopInterval;
                Interval wireBotInterval;

                if ((wTop >= rect.bottom()) && (wTop <= rect.top()))
                {
                    wireTopInterval = Interval{rect.left(), rect.right()};
                }
                if ((wBot >= rect.bottom()) && (wBot <= rect.top()))
                {
                    wireBotInterval = Interval{rect.left(), rect.right()};
                }
                auto commonInterval = wireTopInterval.common(wireBotInterval);
                if (commonInterval.isValid())
                {
                    xIntersections.addInterval(commonInterval);
                }
            }
        }
        std::cout << "x Intersections at y = " << ypos << " :" << xIntersections << "\n";

        // now we check for vertical routing intersections
        CoordType xpos = routingOffset.m_x;
        while(xpos < cellSize.m_x)
        {
            IntervalList yIntersections;
            for(auto const &obj : objs)
            {
                if (std::holds_alternative<ChipDB::Polygon>(obj))
                {
                    // polygon
                }
                else
                {
                    auto const& rect = std::get<ChipDB::Rectangle>(obj);
                    
                    auto wRight = xpos + routingWidth/2;
                    auto wLeft  = xpos - routingWidth/2;

                    Interval wireLeftInterval;
                    Interval wireRightInterval;

                    if ((wRight >= rect.left()) && (wRight <= rect.right()))
                    {
                        wireRightInterval = Interval{rect.left(), rect.right()};
                    }
                    if ((wLeft >= rect.left()) && (wLeft <= rect.right()))
                    {
                        wireLeftInterval = Interval{rect.left(), rect.right()};
                    }

                    auto commonInterval = wireRightInterval.common(wireLeftInterval);
                    if (commonInterval.isValid())
                    {
                        yIntersections.addInterval(commonInterval);
                    }
                }
            }

        }

        ypos += routingPitch.m_y;
    }
}
