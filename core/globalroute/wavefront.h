// SPDX-FileCopyrightText: 2021-2022,2023 Niels Moseley <asicsforthemasses@gmail.com>
//
// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <queue>
#include <vector>
#include "datatypes.h"

namespace LunaCore::GlobalRouter
{

struct WavefrontItem
{
    Predecessor     m_pred{Predecessor::Undefined};
    GCellCoord      m_gridpos;
    PathCostType    m_pathCost{0};
};

class Wavefront
{
public:
    using iterator_type = std::vector<WavefrontItem>::iterator;

    const WavefrontItem& getLowestCostItem() const;

    bool empty() const {return m_items.empty(); }
    void push(const WavefrontItem &item);
    void pop() {m_items.pop(); }
    void clear();

protected:
    
    struct WavefrontItemCompare
    {
        constexpr bool operator()(const WavefrontItem &left, const WavefrontItem &right) const 
        {
            return left.m_pathCost > right.m_pathCost;
        }
    };

    using PQueueType = std::priority_queue<WavefrontItem, std::vector<WavefrontItem>, WavefrontItemCompare>;

    PQueueType m_items;
};

};