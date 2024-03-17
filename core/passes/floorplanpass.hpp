// SPDX-FileCopyrightText: 2021-2024 Niels Moseley <asicsforthemasses@gmail.com>
//
// SPDX-License-Identifier: GPL-3.0-only

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
        registerNamedParameter("coremargins", "", 4, true);
        registerNamedParameter("cornersize", "", 1, true);
    }

    virtual ~Floorplan() = default;

    /** execute a pass given a list of input arguments.
        returns true if succesful, else false.
    */
    [[nodiscard]] bool execute(Database &database) override
    {
        try
        {
            auto widthStr  = m_namedParams.at("width").at(0);
            auto heightStr = m_namedParams.at("height").at(0);

            auto leftStr  = m_namedParams.at("coremargins").at(0);
            auto rightStr  = m_namedParams.at("coremargins").at(1);
            auto topStr    = m_namedParams.at("coremargins").at(2);
            auto bottomStr = m_namedParams.at("coremargins").at(3);

            ChipDB::Coord64 coreSize;
            coreSize.m_x = std::stold(widthStr);
            coreSize.m_y = std::stold(heightStr);

            database.m_design.m_floorplan->setCoreSize(coreSize);

            ChipDB::Margins64 io2coreMargins(
                std::stold(topStr),
                std::stold(bottomStr),
                std::stold(leftStr),
                std::stold(rightStr)
            );

            database.m_design.m_floorplan->setIO2CoreMargins(io2coreMargins);

            if (m_namedParams.contains("cornersize"))
            {
                auto cornerDimension  = std::stold(m_namedParams.at("cornersize").at(0));

                ChipDB::Margins64 cornerMargins(
                    cornerDimension,
                    cornerDimension,
                    cornerDimension,
                    cornerDimension
                );

                database.m_design.m_floorplan->setIOMargins(cornerMargins);
            }
            else
            {
                // Find corner cells and set the size of the io margins that way
                auto ioMarginsOpt = findIOMarginsBasedOnCornerCells();
                if (!ioMarginsOpt)
                {
                    Logging::logWarning("Cannot find any IO corner cells to set the padring margins.\n");
                }
            }
        }
        catch(const std::exception& e)
        {
            Logging::logError("Cannot parse one of the parameter numbers.\n");
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
        ss << "    -width       : the width of the core in nm               [required]\n";
        ss << "    -height      : the height of the core in nm              [required]\n";
        ss << "    -coremargins : the margins between core and io cells     [required]\n";
        ss << "                   <left> <right <top> <bottom> in nm\n";
        ss << "    -cornersize  : dimension of the IO corner in nm          [optional]\n";
        return ss.str();
    }

    /**
        returns a one-line short help text for a pass.
    */
    virtual std::string shortHelp() const noexcept
    {
        return "create a floorplan";
    }

    /**
        Initialize a pass. this is called by registerPass()
    */
    bool init() override
    {
        return true;
    }

protected:

    std::optional<ChipDB::Margins64> findIOMarginsBasedOnCornerCells() const
    {
        return std::nullopt;
    }
};


};
