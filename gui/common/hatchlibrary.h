// SPDX-FileCopyrightText: 2021-2022 Niels Moseley <asicsforthemasses@gmail.com>
//
// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <vector>
#include <QPixmap>

namespace GUI
{

struct HatchLibrary
{
    HatchLibrary();
    std::vector<QPixmap> m_hatches;
};

};
