#pragma once

#include <istream>
#include "netlist/design.h"

/** Namespace for the LEF importers and exporters */
namespace ChipDB::LEF
{

class Reader
{
public:
    static bool load(Design *design, std::istream &source);
};

};
