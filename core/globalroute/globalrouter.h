// SPDX-FileCopyrightText: 2021-2022 Niels Moseley <asicsforthemasses@gmail.com>
//
// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <optional>
#include <utility>
#include "design/design.h"

namespace LunaCore::GlobalRouter
{

class Router
{
public:
    Router() = default;
    
    std::optional<ChipDB::Size64> determineGridCellSize(const ChipDB::Design &design, 
        const std::string &siteName,
        int hRoutes, int vRoutes) const;

    struct Tracks
    {
        int horizontal;
        int vertical;
    };

    std::optional<Tracks> calcNumberOfTracks(const ChipDB::Design &design,
        const std::string &siteName,
        const ChipDB::Size64 &extents) const;

protected:

};

};