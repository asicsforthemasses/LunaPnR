#include "common/logging.h"
#include "cts.h"

bool LunaCore::CTS::doStuff(const std::string &clockNetName, ChipDB::Netlist &netlist)
{
    auto clockNet = netlist.lookupNet(clockNetName);
    if (!clockNet.isValid())
    {
        Logging::doLog(Logging::LogType::ERROR, "CTS cannot find the specified clock net %s\n", clockNetName.c_str());
        return false;
    }

    for(auto const& conn : *clockNet)
    {
    }

    return true;
}
