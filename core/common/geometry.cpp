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
        os << "(empty)";
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

std::ostream& operator<<(std::ostream& os, const ChipDB::Rectangle& rect)
{
    os << rect.m_rect;
    return os;
}

std::ostream& operator<<(std::ostream& os, const ChipDB::Polygon& poly)
{
    std::size_t count = poly.m_points.size();
    if (count == 0)
    {
        os << "(empty)";
        return os;
    }

    os << "poly: ";

    for(auto const& p : poly.m_points)
    {
        os << p;
        if (count != 1)
        {
            os << "->";
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
    ChipDB::Rect64 wireBox;
    bool vertical = true;
    if (vertical)
    {
        // vertical routing wires
        ChipDB::CoordType xpos = routingOffset.m_x;
        while(xpos < cellSize.m_x)
        {
            ChipDB::CoordType ypos = 0;
            const auto w2 = routingWidth/2;
            wireBox = ChipDB::Rect64{{xpos - w2, ypos},{xpos + w2, ypos + cellSize.m_y}};
            
            for(auto const &obj : objs)
            {        
                auto wire = ChipDB::Rectangle{wireBox};
                if (std::holds_alternative<ChipDB::Rectangle>(obj))
                {
                    auto const& rect = std::get<ChipDB::Rectangle>(obj);
                    auto overlap = wire.intersect(rect);
                    if (overlap)
                    {
                        // check that the pin has a minimum size
                        auto const pinSize = overlap.value().getSize();
                        if ((pinSize.m_x >= routingWidth) && (pinSize.m_y >= routingWidth))
                        {
                            std::cout << "overlap: " << overlap.value() << " size: " << pinSize << "\n";
                        }
                    }
                }
                else if (std::holds_alternative<ChipDB::Polygon>(obj))
                {
                    //TODO: polygon. check if we can promote to rectangle.
                    auto const& poly = std::get<ChipDB::Polygon>(obj);
                    std::cout << "findPinLocations: encountered unsupported Polygon: " << poly << "\n";
                    return;
                }
                else
                {
                    std::cout << "findPinLocations: invalid variant object\n";
                    return;
                }
            }
            xpos += routingPitch.m_x;
        }
    }
    else
    {
        // horizontal routing wires
        ChipDB::CoordType ypos = routingOffset.m_y;
        while(ypos < cellSize.m_y)
        {
            auto xpos = 0;
            const auto h2 = routingWidth/2;
            wireBox = ChipDB::Rect64{{xpos, ypos - h2},{xpos + cellSize.m_x, ypos + h2}};
            
            for(auto const &obj : objs)
            {        
                auto wire = ChipDB::Rectangle{wireBox};
                if (std::holds_alternative<ChipDB::Rectangle>(obj))
                {
                    auto const& rect = std::get<ChipDB::Rectangle>(obj);
                    auto overlap = wire.intersect(rect);
                    if (overlap)
                    {
                        // check that the pin has a minimum size
                        auto const pinSize = overlap.value().getSize();
                        if ((pinSize.m_x >= routingWidth) && (pinSize.m_y >= routingWidth))
                        {
                            std::cout << "overlap: " << overlap.value() << " size: " << pinSize << "\n";
                        }
                    }
                }
                else if (std::holds_alternative<ChipDB::Polygon>(obj))
                {
                    //TODO: polygon. check if we can promote to rectangle.
                    auto const& poly = std::get<ChipDB::Polygon>(obj);
                    std::cout << "findPinLocations: encountered unsupported Polygon: " << poly << "\n";
                    return;
                }
                else
                {
                    std::cout << "findPinLocations: invalid variant object\n";
                    return;
                }
            }
            ypos += routingPitch.m_y;
        }        
    }
}
