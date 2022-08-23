// SPDX-FileCopyrightText: 2021-2022 Niels Moseley <asicsforthemasses@gmail.com>
//
// SPDX-License-Identifier: GPL-3.0-only

#pragma once
#include <vector>
#include <variant>
#include "dbtypes.h"

namespace ChipDB
{

class Rectangle
{
public:
    Rectangle(const Rect64 &r) : m_rect(r) {};

    ChipDB::Rect64 m_rect;
};

class Polygon
{
public:
    Polygon(const std::vector<Coord64> &points) : m_points(points) {};

    std::vector<ChipDB::Coord64> m_points;
};

using GeometryObject  = std::variant<Rectangle, Polygon>;
using GeometryObjects = std::vector<GeometryObject>;

};
