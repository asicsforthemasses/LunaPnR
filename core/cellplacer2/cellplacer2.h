#pragma once
#include <unordered_map>
#include "design/design.h"
#include "matrix.h"

namespace LunaCore::CellPlacer2
{

/** place the top module */
bool place(ChipDB::Design &design);

using NetId  = ChipDB::NetObjectKey;
using GateId = ChipDB::InstanceObjectKey;

struct Gate
{
    ChipDB::Coord64         m_size;                 ///< gate size in nm
    std::vector<GateId>     m_netIDs;    ///< connected nets
    ChipDB::Coord64         m_pos;                  ///< location of lower left coordinate
    ChipDB::PlacementInfo   m_placement;
};

struct Net
{
    std::vector<GateId> m_gateIDs;   ///< gates on this net
    float               m_weight{1.0f}; ///< net weight
};

struct PointF
{
    using Type = float;
    Type m_x;
    Type m_y;
};

struct PlacementRegion
{
    ChipDB::Rect64 m_rect;
    double         m_capacity;

    constexpr ChipDB::CoordType width() const noexcept
    {
        return m_rect.width();
    }

    constexpr ChipDB::CoordType height() const noexcept
    {
        return m_rect.height();
    }

    bool contains(const PointF &p) const noexcept
    {
        using Type = PointF::Type;
        if (static_cast<Type>(m_rect.m_ll.m_x) > p.m_x) return false;
        if (static_cast<Type>(m_rect.m_ur.m_x) <= p.m_x) return false;
        if (static_cast<Type>(m_rect.m_ll.m_y) > p.m_y) return false;
        if (static_cast<Type>(m_rect.m_ur.m_y) <= p.m_y) return false;
        return true;
    }

    /** check if point p is inside the region, taking into account an
     *  expansion outward of the region boundary by 'absTol' units.
    */
    bool contains(const PointF &p, const float absTol) const noexcept
    {
        using Type = PointF::Type;
        if ((static_cast<Type>(m_rect.m_ll.m_x)-absTol) > p.m_x) return false;
        if ((static_cast<Type>(m_rect.m_ur.m_x)+absTol) <= p.m_x) return false;
        if ((static_cast<Type>(m_rect.m_ll.m_y)-absTol) > p.m_y) return false;
        if ((static_cast<Type>(m_rect.m_ur.m_y)+absTol) <= p.m_y) return false;
        return true;
    }    

    constexpr auto center() const noexcept
    {
        return m_rect.center();
    }

    int m_level{0}; ///< subdivision level

    std::vector<GateId> m_gatesInRegion;
};

class Placer
{
public:

protected:
    using GateToRowContainer = std::unordered_map<GateId, Matrix::RowIndex>;

    void place(const ChipDB::Netlist &netlist, PlacementRegion &region);

    [[nodiscard]] Matrix::RowIndex findRowOfGate(const GateToRowContainer &gate2Row,
        const GateId gateId) const noexcept;

    /** assign each instance/gate a row in the quadratic placement matrix */
    void mapGatesToMatrixRows(const ChipDB::Netlist &netlist,
        const PlacementRegion &r,
        GateToRowContainer &gate2Row);
};

};
