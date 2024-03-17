#pragma once

#include <string_view>
#include <filesystem>
#include "common/fileutils.h"
#include "common/logging.h"

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
    Database()
    {
        auto pdkInstallPath = getEnvironmentVar("LUNAPNR_PDKINSTALLPATH");
        if (!pdkInstallPath.empty())
        {
            if (std::filesystem::is_directory(pdkInstallPath))
            {
                m_PDKInstallPath = pdkInstallPath;
            }
            else
            {
                Logging::logWarning("LUNAPNR_PDKINSTALLPATH does not point to a valid directory - keeping the default setting.\n");
            }
        }
        else
        {
            Logging::logWarning("LUNAPNR_PDKINSTALLPATH not set - keeping the default setting.\n");
        }
    };

    ChipDB::Design      m_design;
    ChipDB::Properties  m_properties;   ///< global properties, such as paths etc.

    std::filesystem::path m_PDKInstallPath{"/opt/lunapnr/pdkinstalls"};

    constexpr static const char *propPDKRoot{"PDKROOT"};         ///< property key for full path to the currently selected PDK
    constexpr static const char *propProjectRoot{"PROJECTROOT"}; ///< property key for full path to the project directory
};

};
