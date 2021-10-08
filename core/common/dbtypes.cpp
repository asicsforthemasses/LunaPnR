#include "dbtypes.h"

#include <iostream>
#include <array>
#include "dbtypes.h"

std::string ChipDB::toUpper(const std::string &txt)
{
    std::string result = txt;
    for(auto &c : result)
    {
        c = std::toupper(c);
    }
    return result;
}

ChipDB::SymmetryFlags& operator+=(ChipDB::SymmetryFlags &lhs, const uint8_t &rhs)
{
    lhs.m_flags |= rhs;
    return lhs;
}

std::ostream& operator<<(std::ostream& os, const ChipDB::Coord64& r)
{
    os << "(" << r.m_x << "," << r.m_y << ")";
    return os;
}

std::ostream& operator<<(std::ostream& os, const ChipDB::Rect64& r)
{
    os << r.m_ll << "-" << r.m_ur;
    return os;    
}

#if 0

bool ChipDB::isInsideRect(const ChipDB::Coord64 &p, const ChipDB::Rect64 &r) noexcept
{
    bool inx = ((p.m_x >= r.m_ll.m_x) && (p.m_x <= r.m_ur.m_x));
    if (!inx)
        return false;

    bool iny = ((p.m_y >= r.m_ll.m_y) && (p.m_y <= r.m_ur.m_y));
        if (!iny)
        return false;

    return true;
}


bool ChipDB::intersects(const ChipDB::Rect64 &p, const ChipDB::Rect64 &q) noexcept
{
    if ((p.m_ur.m_x < q.m_ll.m_x) || 
        (q.m_ur.m_x < p.m_ll.m_x) ||
        (p.m_ur.m_y < q.m_ll.m_y) ||
        (q.m_ur.m_y < p.m_ll.m_y))
        return false;

    return true;
}

ChipDB::Rect64 ChipDB::unionRect(const ChipDB::Rect64 &r1, const ChipDB::Rect64 &r2) noexcept
{
    ChipDB::Rect64 result;
    result.m_ll.m_x = std::max(r1.m_ll.m_x, r2.m_ll.m_x);
    result.m_ur.m_x = std::min(r1.m_ur.m_x, r2.m_ur.m_x);
    result.m_ll.m_y = std::max(r1.m_ll.m_y, r2.m_ll.m_y);
    result.m_ur.m_y = std::min(r1.m_ur.m_y, r2.m_ur.m_y);    
    return std::move(result);
}
#endif

std::string ChipDB::toString(const CellSubclass &v)
{
    constexpr const std::array<const char*, 21> names = 
    {{
        "NONE", "BUMP", "BLACKBOX", "SOFT", "INPUT", "OUTPUT",
        "INOUT", "POWER", "SPACER", "AREAIO", "FEEDTHRU",
        "TIEHIGH", "TIELOW", "ANTENNACELL", "WELLTAP",
        "PRE", "POST","TOPLEFT", "TOPRIGHT", "BOTTOMLEFT",
        "BOTTOMRIGHT"
    }};

    auto index = static_cast<size_t>(v);
    if (index < names.size())
    {
        return std::string(names[index]);
    }
    else
    {
        return std::string("?");
    }
}

std::string ChipDB::toString(const CellClass &v)
{
    constexpr const std::array<const char*, 6> names = 
    {{
        "CORE", "COVER", "RING", "PAD", "ENDCAP", "BLOCK"
    }};

    auto index = static_cast<size_t>(v);
    if (index < names.size())
    {
        return std::string(names[index]);
    }
    else
    {
        return std::string("?");
    }
}
