#pragma once
#include "common/logging.h"
#include "database/database.h"
#include "pass.hpp"

namespace LunaCore::Passes
{

class Floorplan : public Pass
{
public:
    Floorplan() : Pass("floorplan") {}

    virtual ~Floorplan() = default;

    /** execute a pass given a list of input arguments.
        returns true if succesful, else false.
    */
    [[nodiscard]] bool execute(Database &database, const ArgList &args) override
    {
        processParameters(args);

        for(auto const& param : m_namedParams)
        {
            Logging::doLog(Logging::LogType::INFO, "  %s=%s\n", param.first.c_str(), param.second.c_str());
        }

        for(auto const& param : m_params)
        {
            Logging::doLog(Logging::LogType::INFO, "  %s\n", param.c_str());
        }

        return true;
    }

    /**
        returns help text for a pass.
    */
    std::string help() const noexcept override
    {
        std::stringstream ss;
        ss << "Not help available for " << m_name << "\n";
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
