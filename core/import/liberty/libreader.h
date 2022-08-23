// SPDX-FileCopyrightText: 2021-2022 Niels Moseley <asicsforthemasses@gmail.com>
//
// SPDX-License-Identifier: GPL-3.0-only


#pragma once

#include <istream>
#include "design/design.h"

/** Namespace for the Liberty timing file importers */
namespace ChipDB::Liberty
{

class Reader
{
public:
    static bool load(Design &design, std::istream &source);
};

};