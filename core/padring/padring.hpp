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
        m_size = ChipDB::Size64{-1,-1};
        m_pos  = ChipDB::Coord64();
        m_itemType = ItemType::UNDEFINED;
    }

    std::string         m_instanceName;
    std::string         m_cellName;
    ChipDB::Orientation m_orientation{ChipDB::Orientation::UNDEFINED};
    ChipDB::Size64      m_size;
    ChipDB::Coord64     m_pos;
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

    Layout() = default;

    void clear()
    {
        m_items.clear();
        m_direction = Direction::UNDEFINED;
    }

    /** insert a layout item. Layout takes object ownership */
    void insertLayoutItem(LayoutItem *item)
    {
        m_items.emplace_back(item);
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
            m_items.erase(iter);
            return true;
        }
        return false;
    }

protected:
    Direction m_direction{Direction::UNDEFINED};
    std::list<std::unique_ptr<LayoutItem> > m_items;

    ChipDB::Rect64 m_layoutRect;    ///< the size of the area to use for layout
};

class Padring
{
public:
    Padring() = default;

    void clear();

    Layout m_top;       ///< top layout without the corner cells
    Layout m_bottom;    ///< bottom layout without the corner cells
    Layout m_left;      ///< left layout without the corner cells
    Layout m_right;     ///< right layout without the corner cells

    LayoutItem m_upperLeftCorner;
    LayoutItem m_upperRightCorner;
    LayoutItem m_lowerLeftCorner;
    LayoutItem m_lowerRightCorner;
};

};
