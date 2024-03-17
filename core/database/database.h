#pragma once

#include <string_view>
#include "dbtypes.h"
#include "properties.hpp"
#include "observer.h"
#include "table.hpp"
#include "namedstorage.h"
#include "net.h"
#include "instance.h"
#include "netlist.h"
#include "enums.h"
#include "netlisttools.h"
#include "techlib.h"
#include "floorplan.h"
#include "design.h"

namespace LunaCore
{

struct Database
{
    ChipDB::Design      m_design;
    ChipDB::Properties  m_properties;   ///< global properties, such as paths etc.

    constexpr static const std::string_view propPDKRoot{"PDKROOT"};
    constexpr static const std::string_view propProjectRoot{"PROJECTROOT"};
};

};
