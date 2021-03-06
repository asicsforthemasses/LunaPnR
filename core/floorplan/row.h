/*
  LunaPnR Source Code
  
  SPDX-License-Identifier: GPL-3.0-only
  SPDX-FileCopyrightText: 2022 Niels Moseley <asicsforthemasses@gmail.com>
*/

#pragma once

#include <vector>
#include <memory>
#include "common/dbtypes.h"

namespace ChipDB
{

// predeclarations
struct Region;
class InstanceBase;

enum class RowType
{
    NORMAL,     ///< regular row: cells are placed as-is
    FLIPY       ///< cells are placed vertically flipped
};

struct Row
{
    Row() : m_rowType(RowType::NORMAL), m_region(nullptr) {}

    RowType     m_rowType;  ///< orientation of the row
    Rect64      m_rect;     ///< location and size of the row relative to the region.

    std::shared_ptr<Region> m_region;  ///< the region this row belongs to.
};

};