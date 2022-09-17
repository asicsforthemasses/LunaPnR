#include <algorithm>
#include "geometry.h"

using namespace ChipDB;

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
        auto merged = Interval{std::min(v1.x1, v2.x1), std::max(v1.x2, v2.x2)};
        iter = m_intervals.erase(iter);
        while ((iter != m_intervals.end()) && (merged.overlap(*iter)))
        {
            v2 = *iter;
            merged = Interval{std::min(merged.x1, v2.x1), std::max(merged.x2, v2.x2)};
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
