/*
  LunaPnR Source Code
  
  SPDX-License-Identifier: GPL-3.0-only
  SPDX-FileCopyrightText: 2022 Niels Moseley <asicsforthemasses@gmail.com>
*/


#pragma once

#include <istream>
#include "design/design.h"

/** Namespace for the LEF importers and exporters */
namespace ChipDB::LEF
{

class Reader
{
public:
    static bool load(Design &design, std::istream &source);
};

};
