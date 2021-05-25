#pragma once

#include <istream>
#include "netlist/design.h"

/** Namespace for the Liberty timing file importers */
namespace ChipDB::Liberty
{

class Reader
{
public:
    static bool load(Design *design, std::istream &source);
};

};