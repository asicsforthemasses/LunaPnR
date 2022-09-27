#include "prim_private.h"

bool LunaCore::Prim::Private::operator<(
    const LunaCore::Prim::Private::CostTuple &lhs, 
    const LunaCore::Prim::Private::CostTuple &rhs) noexcept
{
    if (lhs.m_dist < rhs.m_dist) return true;
    if (lhs.m_dist > rhs.m_dist) return false;
    
    // lhs.m_dist == rhs.m_dist here.
    if (lhs.m_minAbsY < rhs.m_minAbsY) return true;
    if (lhs.m_minAbsY > rhs.m_minAbsY) return false;

    // lhs.m_dist == rhs.m_dist here.
    // and lhs.m_minAbsY == rhs.m_minAbsY here.
    return (lhs.m_minMaxX < rhs.m_minMaxX);
}
