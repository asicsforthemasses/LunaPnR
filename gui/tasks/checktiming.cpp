// SPDX-FileCopyrightText: 2021-2022 Niels Moseley <asicsforthemasses@gmail.com>
//
// SPDX-License-Identifier: GPL-3.0-only

#include "checktiming.h"
#include "common/logging.h"
#include "common/subprocess.h"
#include "opensta/openstaparser.h"

void Tasks::CheckTiming::execute(GUI::Database &database, ProgressCallback callback)
{
    // check if the is a top module set. If so, use that
    // otherwise check if there is just one module
    // and use that.

    auto topModule = database.design().getTopModule();

    if (!topModule)
    {
        error("Top module not set\n");
        return;
    }

    info(Logging::fmt("Checking timing using module: %s\n", topModule->name().c_str()));

    auto spefTempFile = ChipDB::createTempFile("spef");
    if (m_mode == Mode::WITHSPEF)
    {
        // create SPEF file
        info("Creating SPEF file..\n");
        LunaCore::SPEF::write(spefTempFile->m_stream, topModule);
    }

    auto tclContents = createTCL(database, topModule->name(), spefTempFile->m_name);

    // create a temporary file to give to OpenSTA
    auto tclFileDescriptor = ChipDB::createTempFile("tcl");

    if (!tclFileDescriptor->good())
    {
        error("Cannot create temporary TCL file\n");
        return;
    }

    tclFileDescriptor->m_stream << tclContents;
    tclFileDescriptor->close(); // closes but doesn't delete the file.

    std::stringstream cmd;
    cmd << database.m_projectSetup.m_openSTALocation;
    cmd << " -no_splash -exit " << tclFileDescriptor->m_name << "\n";

    info(Logging::fmt("Running %s\n", cmd.str().c_str()));

    GUI::OpenSTAParser parser;

    auto lineCallback = [this, &parser](const std::string& line)
    {
        if (!parser.submitLine(line))
        {
            error("OpenSTA parser error:\n");
        }
        info(line);
    };

    if (!ChipDB::Subprocess::run(cmd.str(), lineCallback))
    {
        error("OpenSTA failed\n");
        return;
    }
    else
    {   
        info("OpenSTA ok\n");
    }

    auto timeUnits = parser.timeUnits();

    // report the paths
    info("** Path report **");
    std::size_t pathsReported = 0;
    std::size_t warnings = 0;
    bool timingErrors = false;
    
    for(auto iter = parser.beginPaths(); iter != parser.endPaths(); ++iter)
    {
        if (iter->m_slack >= 0.0)
        {
            info(Logging::fmt("  From %s to %s -> slack %f %s\n", 
                iter->m_source.c_str(), iter->m_destination.c_str(), 
                iter->m_slack * timeUnits.first,
                timeUnits.second.c_str()));
        }
        else
        {
            error(Logging::fmt("  From %s to %s -> slack %f %s  FAILED\n", 
                iter->m_source.c_str(), iter->m_destination.c_str(), 
                iter->m_slack * timeUnits.first, timeUnits.second.c_str()));
            timingErrors = true;
        }
        pathsReported++;
    }

    if (pathsReported == 0)
    {
        warning("  No paths to report - are you sure your timing constraints are setup correctly?\n");
        warnings++;
    }

    if (!parser.setupWarnings().empty())
    {
        warning("** Timing analysis found the following issues with your setup **\n");
        for(auto warningText : parser.setupWarnings())
        {
            warning(warningText);
            warnings++;
        }
    }

    if (timingErrors) 
    {
        error("** Timing checks failed **\n");
        return;
    }

    if (warnings != 0)
    {
        warning(Logging::fmt("Timing analysis reports %d warnings!\n", warnings));
    }
    else
    {
        info("** Timing checks passed **\n");
    }

    done();
}

std::string Tasks::CheckTiming::createTCL(const GUI::Database &database, 
    const std::string &topModuleName,
    const std::string &spefFilename) const
{
    std::stringstream tcl;

    for(auto const& lib : database.m_projectSetup.m_libFiles)
    {
        tcl << "read_liberty " << lib << "\n";
    }

    for(auto const& verilog : database.m_projectSetup.m_verilogFiles)
    {
        tcl << "read_verilog " << verilog << "\n";
    }

    tcl << "link_design " << topModuleName << "\n";


    if (m_mode == Mode::WITHSPEF)
    {
        tcl << "read_spef " << spefFilename << "\n";
    }

    for(auto const& sdc : database.m_projectSetup.m_timingConstraintFiles)
    {
        tcl << "read_sdc " << sdc << "\n";
    }    

    tcl << R"(puts "#UNITS")" "\n";
    tcl << "report_units\n";
    tcl << R"(puts "#CHECKSETUP")" "\n";
    tcl << "check_setup\n";
    tcl << R"(puts "#REPORTCHECKS")" "\n";
    tcl << "report_checks\n";

    return tcl.str();
}

