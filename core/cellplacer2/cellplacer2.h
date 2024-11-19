// SPDX-FileCopyrightText: 2021-2024 Niels Moseley <asicsforthemasses@gmail.com>
//
// SPDX-License-Identifier: GPL-3.0-only

#pragma once
#include <unordered_map>
#include <deque>
#include <memory>

#if 0
#include <Eigen/Core>
#include <Eigen/Dense>
#include <Eigen/Sparse>
#include <Eigen/IterativeLinearSolvers>
#endif

#include "database/database.h"
#include "algebra/sparsematrix.hpp"

namespace LunaCore::CellPlacer2
{

using NetId  = ChipDB::NetObjectKey;
using GateId = ChipDB::InstanceObjectKey;

struct PointF
{
    using Type = float;

    constexpr PointF() = default;
    constexpr PointF(const Type &x, const Type &y) : m_x(x), m_y(y) {}

    constexpr PointF(const ChipDB::Coord64 &pos)
        : m_x{static_cast<Type>(pos.m_x)},
          m_y{static_cast<Type>(pos.m_y)}
        {}

    Type m_x{0};
    Type m_y{0};

    [[nodiscard]] constexpr ChipDB::Coord64 toCoord64() const noexcept
    {
        return {static_cast<ChipDB::CoordType>(m_x), static_cast<ChipDB::CoordType>(m_y)};
    }
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

    /** place the movable cells/gates/instances of the netlist inside the
     *  ChipDB::Region using quadratic placement.
     *
     *  The chip is recursively divided into subregions. maxLevels sets the
     *  maximum number of divisions to use.
     *
     *  Recursive subregion division also stops when a subregion contains
     *  fewer than 'minInstances' cells/gates/instances.
    */
    [[nodiscard]] bool place(ChipDB::Netlist &netlist, ChipDB::Floorplan &floorplan,
        std::size_t maxLevels, std::size_t minInstances);

protected:
    using GateToRowContainer = std::unordered_map<GateId, Matrix::RowIndex>;
    using GatePosContainer   = std::unordered_map<GateId, PointF>;

    /** place the cells/gates/instances in the PlacementRegion using
     *  quadratic placement and update the location of them in the netlist instances.
     *  cells/gates/instances outside the PlacementRegion are not affected.
    */
    void placeRegion(ChipDB::Netlist &netlist, PlacementRegion &region);

    void cycle(ChipDB::Netlist &netlist, std::deque<std::unique_ptr<PlacementRegion>> &regions);

    [[nodiscard]] Matrix::RowIndex findRowOfGate(const GateToRowContainer &gate2Row,
        const GateId gateId) const noexcept;

    [[nodiscard]] PointF propagate(const PlacementRegion &region, const PointF &p) const;

    /** assign each instance/gate a row in the quadratic placement matrix */
    void mapGatesToMatrixRows(const ChipDB::Netlist &netlist,
        const PlacementRegion &r,
        GateToRowContainer &gate2Row);

    void populateGatePositions(const ChipDB::Netlist &netlist, ChipDB::Floorplan &floorplan);

    enum class Direction
    {
        HORIZONTAL,
        VERTICAL
    };

    void sortGates(PlacementRegion &region, Direction dir);
    void cutRegion(const PlacementRegion &region, Direction dir,
        PlacementRegion &region1, PlacementRegion &region2) const;

    GatePosContainer m_gatePositions;
    std::size_t m_maxLevels{0};
    std::size_t m_minInstancesInRegion{0};
};

};


std::ostream& operator<<(std::ostream &os, const LunaCore::CellPlacer2::PointF &p);
std::ostream& operator<<(std::ostream &os, const LunaCore::CellPlacer2::PlacementRegion &r);
