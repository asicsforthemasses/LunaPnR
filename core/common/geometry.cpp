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

std::ostream& operator<<(std::ostream& os, const ChipDB::Interval& v)
{
    os << v.x1 << ".." << v.x2;
    return os;    
}
