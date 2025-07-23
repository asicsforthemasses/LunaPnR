// SPDX-FileCopyrightText: 2021-2025 Niels Moseley <asicsforthemasses@gmail.com>
//
// SPDX-License-Identifier: GPL-3.0-only

#pragma once
#include <optional>
#include "database/database.h"

namespace LunaCore
{

/** Specification for the 3-point L shape configuration */
enum class LShape
{
    UNKNOWN = 0,
    UPPER,
    LOWER,
    NONE
};

/** generate the optional 3rd "Steiner" point for an L shape, given two corners of the bounding box. */
constexpr std::optional<ChipDB::Coord64> LSteinerPoint(const ChipDB::Coord64 &p1, const ChipDB::Coord64 &p2, LShape shape) noexcept
{
    // there is no L-shape / Steiner point when the points
    // lie on the same horizontal or vertical line.
    if ((p1.m_x == p2.m_x) || (p1.m_y == p2.m_y))
    {
        return std::nullopt;
    }

    switch(shape)
    {
    case LShape::UPPER:
        {
            if (p1.m_y > p2.m_y)
            {
                return ChipDB::Coord64{p2.m_x, p1.m_y};
            }
            else
            {
                return ChipDB::Coord64{p1.m_x, p2.m_y};
            }
        }
    case LShape::LOWER:
        {
            if (p1.m_y > p2.m_y)
            {
                return ChipDB::Coord64{p1.m_x, p2.m_y};
            }
            else
            {
                return ChipDB::Coord64{p2.m_x, p1.m_y};
            }
        }
    case LShape::NONE:
    case LShape::UNKNOWN:
        break;
    }

    return std::nullopt;
};

};