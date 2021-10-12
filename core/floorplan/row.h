#pragma once

#include "common/dbtypes.h"

namespace ChipDB
{

// predeclaration
struct Region;

struct Row
{
    Rect64 m_rect;      ///< location and size of the row relative to the region.
    Region *m_region;   ///< the region this row belongs to.
};

};