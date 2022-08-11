#include <sstream>
#include <fstream>
#include "readallfiles.h"

void Tasks::ReadAllFiles::execute(GUI::Database &database)
{
    m_status.store(Status::RUNNING);
        
    for(auto const& lef : database.m_projectSetup.m_lefFiles)
    {
        std::ifstream lefFile(lef);
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
        std::ifstream libFile(lib);
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

    for(auto const& verilog : database.m_projectSetup.m_verilogFiles)
    {
        std::ifstream verilogFile(verilog);
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
