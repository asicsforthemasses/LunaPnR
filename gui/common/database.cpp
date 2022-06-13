// SPDX-FileCopyrightText: 2021-2022 Niels Moseley, <n.a.moseley@moseleyinstruments.com>, et al.
//
// SPDX-License-Identifier: GPL-3.0-only

#include "database.h"

using namespace GUI;

void Database::clear()
{
    m_design.clear();
    m_layerRenderInfoDB.clear();    
}
