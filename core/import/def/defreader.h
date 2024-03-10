// SPDX-FileCopyrightText: 2021-2024 Niels Moseley <asicsforthemasses@gmail.com>
//
// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <istream>
#include "database/database.h"

/** Namespace for the DEF importers and exporters */
namespace ChipDB::DEF
{

class Reader
{
public:
    /** loads placement information from the DEF file into the database.
     *  The database needs to contain a valid netlist and all
     *  cells/instances referenced in the DEF file must
     *  exist.
    */
    static bool load(Design &design, std::istream &source);
};

};
