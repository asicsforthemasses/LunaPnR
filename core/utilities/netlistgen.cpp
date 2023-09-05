//  LunaPnR Source Code
//
//  SPDX-License-Identifier: GPL-3.0-only
//  SPDX-FileCopyrightText: 2022,2023 Niels Moseley <asicsforthemasses@gmail.com>
//  

#include <lunacore.h>
#include <cxxopts.hpp>
#include <string>
#include <vector>
#include <iostream>

int main(int argc, const char* argv[])
{
    cxxopts::Options options("netlistgen", "A Verilog/LEF to simple netlist generator");

    options.add_options()
        ("h,help", "Show help")
        ("verilog", "Verilog netlist file", cxxopts::value<std::string>())
        ("lef", "LEF file", cxxopts::value<std::vector<std::string>>())
        ("o,out", "Output generic netlist file", cxxopts::value<std::string>())
        ("v,verbose", "Verbose output", cxxopts::value<bool>()->default_value("false"))
        ;

    auto result = options.parse(argc, argv);
    bool verbose = result["verbose"].as<bool>();

    if (result.count("help") > 0)
    {
        std::cout << options.help() << "\n";
        return EXIT_FAILURE;
    }

    if (result.count("verilog") != 1)
    {
        std::cout << options.help() << "\n\n";
        std::cout << "Need exactly one Verilog netlist file" << "\n";        
        return EXIT_FAILURE;
    }

    if (result.count("lef") < 1)
    {
        std::cout << options.help() << "\n\n";
        std::cout << "Need one or more LEF files" << "\n";
        return EXIT_FAILURE;
    }

    if (result.count("out") != 1)
    {
        std::cout << options.help() << "\n\n";
        std::cout << "Need exactly one output netlist file" << "\n";
        return EXIT_FAILURE;
    }

    std::cout << "Netlistgen version 0.1\n\n";

    std::string verilogFilename = result["verilog"].as<std::string>();

    auto LEFs = result["lef"].as<std::vector<std::string>>();

    ChipDB::Design design;

    // ********************************************************************************
    // ** LOAD LEFs
    // ********************************************************************************
    for(auto const& lef : LEFs)
    {
        std::ifstream infile(lef);
        if (!infile)
        {            
            std::cerr << "Cannot open " << lef << "\n";
            return EXIT_FAILURE;
        }

        if (!ChipDB::LEF::Reader::load(design, infile))
        {
            std::cerr << "Error reading " << lef << "\n";
            return EXIT_FAILURE;
        }

        std::cout << "  Loaded "  << lef << "\n";
    }

    // ********************************************************************************
    // ** LOAD VERILOG
    // ********************************************************************************

    std::ifstream infile(verilogFilename);
    if (!infile)
    {
        std::cerr << "Cannot open " << verilogFilename << "\n";
        return EXIT_FAILURE;        
    }

    if (!ChipDB::Verilog::Reader::load(design, infile))
    {
        std::cerr << "Error reading " << verilogFilename << "\n";
        return EXIT_FAILURE;
    }

    std::cout << "  Loaded "  << verilogFilename << "\n";

    if (design.m_moduleLib->size() < 1)
    {
        std::cerr << "Design does not contain any modules!\n";
        return EXIT_FAILURE;
    }

    auto mod = *design.m_moduleLib->begin();

    if (verbose)
    {
        std::cout << "  Instances: " << mod->m_netlist->m_instances.size() << "\n";
        std::cout << "  Nets     : " << mod->m_netlist->m_nets.size() << "\n";
    }

    // write to output
    auto outputFilename = result["out"].as<std::string>();
    auto ofile = std::ofstream(outputFilename);
    if (!ofile)
    {
        std::cerr << "Cannot open " << outputFilename << " for writing\n";
        return EXIT_FAILURE;
    }

    // write number of instances
    ofile << mod->m_netlist->m_instances.size() << "\n";
    
    // write number of nets
    ofile << mod->m_netlist->m_nets.size() << "\n";

    // write each net
    for(auto const& netKP : mod->m_netlist->m_nets)
    {
        ofile << netKP->numberOfConnections() << " ";
        for(auto const& conn : *netKP)
        {
            ofile << conn.m_instanceKey << " ";
        }
        ofile << "\n";
    }

    return EXIT_SUCCESS;
}
