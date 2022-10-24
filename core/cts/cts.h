#pragma once
#include <string>
#include "common/dbtypes.h"
#include "netlist/net.h"
#include "design/design.h"

namespace LunaCore::CTS
{

bool doStuff(const std::string &clockNetName, ChipDB::Netlist &netlist);

};