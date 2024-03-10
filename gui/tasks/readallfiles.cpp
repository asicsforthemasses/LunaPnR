// SPDX-FileCopyrightText: 2021-2024 Niels Moseley <asicsforthemasses@gmail.com>
//
// SPDX-License-Identifier: GPL-3.0-only

#include <sstream>
#include <fstream>
#include "readallfiles.h"

void Tasks::ReadAllFiles::execute(GUI::Database &database, ProgressCallback callback)
{
    m_status.store(Status::RUNNING);

    database.clear();

    for(auto const& lef : database.m_projectSetup.m_lefFiles)
    {
        std::ifstream lefFile(ChipDB::expandEnvironmentVars(lef));
        if (!lefFile.good())
        {
            std::stringstream ss;
            ss << "Could not open LEF file " << lef << "\n";
            error(ss.str());
            return;
        }

        std::stringstream ss;
        ss << lefFile.rdbuf();
        if (!ChipDB::LEF::Reader::load(database.design(), ss))
        {
            error("Could not parse LEF file");
            return;
        }
    }

    for(auto const& lib : database.m_projectSetup.m_libFiles)
    {
        std::ifstream libFile(ChipDB::expandEnvironmentVars(lib));
        if (!libFile.good())
        {
            std::stringstream ss;
            ss << "Could not open LIB file " << lib << "\n";
            error(ss.str());
            return;
        }

        std::stringstream ss;
        ss << libFile.rdbuf();
        if (!ChipDB::Liberty::Reader::load(database.design(), ss))
        {
            error("Could not parse LIB file");
            return;
        }
    }

    for(auto const& layerFileName : database.m_projectSetup.m_layerFiles)
    {
        std::ifstream layerFile(ChipDB::expandEnvironmentVars(layerFileName));
        if (!layerFile.good())
        {
            std::stringstream ss;
            ss << "Could not open Layer file " << layerFileName << "\n";
            error(ss.str());
            return;
        }

        std::stringstream ss;
        ss << layerFile.rdbuf();

        if (!database.m_layerRenderInfoDB.readJson(ss.str()))
        {
            error("Could not parse Layer file");
            return;
        }
    }

    for(auto const& verilog : database.m_projectSetup.m_verilogFiles)
    {
        std::ifstream verilogFile(ChipDB::expandEnvironmentVars(verilog));
        if (!verilogFile.good())
        {
            std::stringstream ss;
            ss << "Could not open Verilog file " << verilog << "\n";
            error(ss.str());
            return;
        }

        std::stringstream ss;
        ss << verilogFile.rdbuf();
        if (!ChipDB::Verilog::Reader::load(database.design(), ss))
        {
            error("Could not parse Verilog file");
            return;
        }
    }

    done();
}
