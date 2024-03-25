// SPDX-FileCopyrightText: 2021-2024 Niels Moseley <asicsforthemasses@gmail.com>
//
// SPDX-License-Identifier: GPL-3.0-only

#pragma once
#include <string>
#include <memory>
#include <list>
#include <algorithm>
#include "database/database.h"

namespace LunaCore::Padring
{

class LayoutItem
{
public:
    enum class ItemType
    {
        UNDEFINED,
        CELL,
        CORNER,
        FIXEDSPACE,
        FLEXSPACE,
        FILLER
    };

    LayoutItem() = default;

    void clear()
    {
        m_cellName = "";
        m_instanceName = "";
        m_orientation = ChipDB::Orientation::UNDEFINED;
        m_width  = 0;
        m_height = 0;
        m_pos    = 0;
        m_itemType = ItemType::UNDEFINED;
        m_subClass = ChipDB::CellSubclass::NONE;
    }

    std::string         m_instanceName;
    std::string         m_cellName;
    ChipDB::Orientation m_orientation{ChipDB::Orientation::UNDEFINED};
    ChipDB::CoordType   m_width{0};
    ChipDB::CoordType   m_height{0};
    ChipDB::CoordType   m_pos{0};
    ChipDB::CellSubclass m_subClass{ChipDB::CellSubclass::NONE};    ///< subclass needed for cornercell default orientation
    ItemType m_itemType{ItemType::UNDEFINED};
};

class Layout
{
public:
    enum class Direction
    {
        UNDEFINED,
        HORIZONTAL,
        VERTICAL
    };

    enum class Location
    {
        UNDEFINED,
        TOP,
        BOTTOM,
        LEFT,
        RIGHT
    };

    Layout() = default;

    void clear()
    {
        m_items.clear();
        m_cellCount = 0;
        m_direction = Direction::UNDEFINED;
    }

    [[nodiscard]] constexpr auto cellCount() const noexcept
    {
        return m_cellCount;
    }

    /** insert a layout item. Layout takes object ownership */
    void insertLayoutItem(LayoutItem *item)
    {
        m_items.emplace_back(item);
        if (item->m_itemType == LayoutItem::ItemType::CELL)
        {
            m_cellCount++;
        }
    }

    /** remove a layout item from the list. */
    bool removeLayoutItem(const std::string &instanceName)
    {
        auto iter = std::find_if(m_items.begin(), m_items.end(),
            [&](auto const &ptr)
            {
                return (ptr->m_instanceName == instanceName);
            }
        );

        if (iter != m_items.end())
        {
            if ((*iter)->m_itemType == LayoutItem::ItemType::CELL)
            {
                m_cellCount--;
            }

            m_items.erase(iter);
            return true;
        }
        return false;
    }

    auto begin() const {return m_items.begin(); }
    auto end() const {return m_items.end(); }

    constexpr void setLocation(const Location &loc) noexcept
    {
        m_location = loc;
    }

    constexpr auto location() const noexcept
    {
        return m_location;
    }

    constexpr Direction direction() const noexcept
    {
        switch(m_location)
        {
        case Location::UNDEFINED:
            return Direction::UNDEFINED;
        case Location::LEFT:
        case Location::RIGHT:
            return Direction::VERTICAL;
        case Location::TOP:
        case Location::BOTTOM:
            return Direction::HORIZONTAL;
        }

        return Direction::UNDEFINED;
    }

    constexpr void setLayoutRect(const ChipDB::Rect64 &rect)
    {
        m_layoutRect = rect;
    }

    constexpr auto layoutRect() const noexcept
    {
        return m_layoutRect;
    }

    constexpr void setDefaultCellOrientation(const ChipDB::Orientation &orientation) noexcept
    {
        m_defaultCellOrientation = orientation;
    }

    constexpr auto defaultCellOrientation() const noexcept
    {
        return m_defaultCellOrientation;
    }

    /** pad alignment for when corner cells are larger than pads */
    enum class PadAlignment
    {
        NONE,
        TOP,
        RIGHT
    };

protected:
    Location m_location{Location::UNDEFINED};

    Direction m_direction{Direction::UNDEFINED};
    std::list<std::unique_ptr<LayoutItem> > m_items;

    std::size_t m_cellCount{0};
    ChipDB::Rect64 m_layoutRect;    ///< the size of the area to use for layout

    ChipDB::Orientation m_defaultCellOrientation{ChipDB::Orientation::UNDEFINED};
};

class Padring
{
public:
    Padring() = default;

    void clear();
    bool layout(Database &db);

    Layout m_top;       ///< top layout without the corner cells
    Layout m_bottom;    ///< bottom layout without the corner cells
    Layout m_left;      ///< left layout without the corner cells
    Layout m_right;     ///< right layout without the corner cells

    LayoutItem m_upperLeftCorner;
    LayoutItem m_upperRightCorner;
    LayoutItem m_lowerLeftCorner;
    LayoutItem m_lowerRightCorner;

    constexpr void setDefaultPadOrientation(const ChipDB::Orientation &orientation) noexcept
    {
        m_defaultPadOrientation = orientation;
    }

protected:
    bool layoutEdge(Database &db, const LayoutItem &corner1, const LayoutItem &corner2, const Layout &edge);

    // fill gap using filler cells, from 'from' to 'to' and
    // 'otherAxis' is the coordinate of the axis perpendicular to the from-to axis.
    bool fillGap(
        Database &db,
        const Layout &edge,
        const ChipDB::CoordType from,
        const ChipDB::CoordType to);

    bool placeInstance(Database &db,
        const std::string &insName,
        const ChipDB::Coord64 &lowerLeftPos,
        const ChipDB::Orientation &orientation);

    struct Spacer
    {
        ChipDB::CellObjectKey m_cellKey{ChipDB::ObjectNotFound};
        std::string         m_name;
        ChipDB::Size64      m_size;
        ChipDB::Coord64     m_offset;

        [[nodiscard]] constexpr auto width() const noexcept { return m_size.m_x; }
        [[nodiscard]] constexpr auto height() const noexcept { return m_size.m_y; }
    };

    void findSpacers(Database &db);

    ChipDB::CoordType roundToValidPos(ChipDB::CoordType coord,
        ChipDB::CoordType smallestSpacerWidth,
        ChipDB::CoordType corner1EdgePos) const;

    std::vector<Spacer> m_spacers;
    int32_t             m_fillerCount{0};
    ChipDB::Orientation m_defaultPadOrientation{ChipDB::Orientation{ChipDB::Orientation::R180}};
};

};
