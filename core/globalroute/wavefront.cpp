
// SPDX-FileCopyrightText: 2021-2024 Niels Moseley <asicsforthemasses@gmail.com>
//
// SPDX-License-Identifier: GPL-3.0-only

#include "wavefront.h"
#include <algorithm>

using namespace LunaCore;

const GlobalRouter::WavefrontItem& GlobalRouter::Wavefront::getLowestCostItem() const
{
    return m_items.top();
}

void GlobalRouter::Wavefront::push(const WavefrontItem &item)
{
    m_items.push(item);
}

void GlobalRouter::Wavefront::clear()
{
    m_items = PQueueType{};
}
