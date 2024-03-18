// SPDX-FileCopyrightText: 2021-2024 Niels Moseley <asicsforthemasses@gmail.com>
//
// SPDX-License-Identifier: GPL-3.0-only

#include "padring.hpp"

namespace LunaCore::Padring
{

void Padring::clear()
{
    m_top.clear();
    m_bottom.clear();
    m_left.clear();
    m_right.clear();

    m_lowerLeftCorner.clear();
    m_lowerRightCorner.clear();
    m_upperLeftCorner.clear();
    m_upperRightCorner.clear();

    m_lowerLeftCorner.m_itemType = LayoutItem::ItemType::CORNER;
    m_lowerRightCorner.m_itemType = LayoutItem::ItemType::CORNER;
    m_upperLeftCorner.m_itemType = LayoutItem::ItemType::CORNER;
    m_upperRightCorner.m_itemType = LayoutItem::ItemType::CORNER;
};

};