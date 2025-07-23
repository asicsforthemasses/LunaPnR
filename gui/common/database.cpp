// SPDX-FileCopyrightText: 2021-2025 Niels Moseley <asicsforthemasses@gmail.com>
//
// SPDX-License-Identifier: GPL-3.0-only

#include "database.h"

namespace GUI
{

void Database::clear()
{
    m_coreDatabase.m_design.clear();
    m_layerRenderInfoDB.clear();
}

};
