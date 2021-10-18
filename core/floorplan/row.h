#pragma once

#include "common/dbtypes.h"

namespace ChipDB
{

// predeclaration
struct Region;

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
    Region      *m_region;  ///< the region this row belongs to.
};

};