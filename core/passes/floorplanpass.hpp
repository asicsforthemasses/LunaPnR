#pragma once
#include "common/logging.h"
#include "pass.hpp"

namespace LunaCore::Passes
{

class Floorplan : public Pass
{
public:
    Floorplan() : Pass("floorplan")
    {
        registerNamedParameter("width", "", 1, true);
        registerNamedParameter("height", "", 1, true);
    }

    virtual ~Floorplan() = default;

    /** execute a pass given a list of input arguments.
        returns true if succesful, else false.
    */
    [[nodiscard]] bool execute(Database &database) override
    {
#if 0
        for(auto const& param : m_namedParams)
        {
            Logging::doLog(Logging::LogType::INFO, "  %s\n", param.first.c_str());
            for(auto const& paramArg : param.second)
            {
                Logging::doLog(Logging::LogType::INFO, "      %s\n", paramArg.c_str());
            }
        }

        for(auto const& param : m_params)
        {
            Logging::doLog(Logging::LogType::INFO, "  %s\n", param.c_str());
        }
#endif

        if (m_namedParams.contains("help"))
        {
            Logging::doLog(Logging::LogType::INFO,  help());
            return false;
        }

        return true;
    }

    /**
        returns help text for a pass.
    */
    std::string help() const noexcept override
    {
        std::stringstream ss;
        ss << "Floorplan - create a floorplan\n";
        ss << "  Options:\n";
        ss << "    -width   : the width of the floorplan in nm     [required]\n";
        ss << "    -height  : the height of the floorplan in nm    [required]\n";
        return ss.str();
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
