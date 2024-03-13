#pragma once
#include <fstream>
#include <filesystem>
#include "common/logging.h"
#include "import/import.h"
#include "pass.hpp"

namespace LunaCore::Passes
{

class InfoPass : public Pass
{
public:
    InfoPass() : Pass("info")
    {
        registerNamedParameter("cells", "", 0, false);
        registerNamedParameter("cell", "", 1, false);
    }

    virtual ~InfoPass() = default;

    /** execute a pass given a list of input arguments.
        returns true if succesful, else false.
    */
    [[nodiscard]] bool execute(Database &database) override
    {
        if (m_namedParams.contains("cells"))
        {
            // dump all the cells
            auto cellLibPtr = database.m_design.m_cellLib;
            if (!cellLibPtr)
            {
                Logging::doLog(Logging::LogType::ERROR, "Cell library does not exist\n");
                return false;
            }

            Logging::doLog(Logging::LogType::INFO, "Cells:\n");
            for(auto cellKp : *cellLibPtr)
            {
                Logging::doLog(Logging::LogType::INFO, "    %s\n", cellKp->name().c_str());
            }
        }

        if (m_namedParams.contains("cell"))
        {
            auto cellLibPtr = database.m_design.m_cellLib;
            if (!cellLibPtr)
            {
                Logging::doLog(Logging::LogType::ERROR, "Cell library does not exist\n");
                return false;
            }

            for(auto cellName : m_namedParams.at("cell"))
            {
                auto cellKp = cellLibPtr->lookupCell(cellName);
                Logging::doLog(Logging::LogType::INFO, "    Name   : %s\n", cellKp->name().c_str());
                Logging::doLog(Logging::LogType::INFO, "    Width  : %d nm\n", cellKp->m_size.m_x);
                Logging::doLog(Logging::LogType::INFO, "    Height : %d nm\n", cellKp->m_size.m_y);
                Logging::doLog(Logging::LogType::INFO, "    Pins   : %d\n", cellKp->getNumberOfPins());

                for(auto pin : cellKp->m_pins)
                {
                    Logging::doLog(Logging::LogType::INFO, "        %s\t%s\n", pin->m_name.c_str(), toString(pin->m_iotype).c_str());
                }
            }
        }

        return true;
    }

    /**
        returns help text for a pass.
    */
    std::string help() const noexcept override
    {
        std::stringstream ss;
        ss << "info - query the database\n";
        ss << "  info <info type> [name1 name2 ...]\n\n";
        ss << "  Info type options:\n";
        ss << "    -cells   : show all the cells\n";
        ss << "\n";
        return ss.str();
    }

    /**
        returns a one-line short help text for a pass.
    */
    virtual std::string shortHelp() const noexcept
    {
        return "query the datatbase";
    }

    /**
        Initialize a pass. this is called by registerPass()
    */
    bool init() override
    {
        return true;
    }
};


};
