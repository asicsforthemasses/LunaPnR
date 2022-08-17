#include "checktiming.h"
#include "common/logging.h"
#include "common/subprocess.h"

void Tasks::CheckTiming::execute(GUI::Database &database)
{
    // check if the is a top module set. If so, use that
    // otherwise check if there is just one module
    // and use that.

    auto topModule = database.design().getTopModule();

    if (!topModule)
    {
        if (database.design().m_moduleLib->size() == 1)
        {
            auto moduleIter = database.design().m_moduleLib->begin();
            if (!database.design().setTopModule(moduleIter->name()))
            {
                error("Cannot set top module\n");
                return;
            }
            topModule = database.design().getTopModule();
        }
        else
        {
            error("Cannot deduce top module\n");
            return;
        }
    }

    if (!topModule)
    {
        error("Top module not set\n");
        return;
    }

    info(Logging::fmt("Checking timing using module: %s\n", topModule->name().c_str()));

    auto tclContents = createTCL(database, topModule->name());

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
    cmd << "/usr/local/bin/sta -no_splash -exit " << tclFileDescriptor->m_name << "\n";

    info(Logging::fmt("Running %s\n", cmd.str().c_str()));

    auto lineCallback = [this](const std::string& str)
    {
        info(str);
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
    done();
}


std::string Tasks::CheckTiming::createTCL(const GUI::Database &database, const std::string &topModuleName) const
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

    for(auto const& sdc : database.m_projectSetup.m_timingConstraintFiles)
    {
        tcl << "read_sdc " << sdc << "\n";
    }    

    tcl << "check_setup\n";
    tcl << "report_checks\n";

    return tcl.str();
}
