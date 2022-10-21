#pragma once
#include <string>
#include "common/dbtypes.h"
#include "netlist/net.h"
#include "design/design.h"

namespace LunaCore::CTS
{

/** node holding the instance key and the instance position */
struct Node
{
    ChipDB::ObjectKey m_insKey;
    ChipDB::Coord64   m_pos;
};

bool doStuff(const std::string &clockNetName, ChipDB::Netlist &netlist);

};