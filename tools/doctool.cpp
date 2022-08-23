// SPDX-FileCopyrightText: 2021-2022 Niels Moseley <asicsforthemasses@gmail.com>
//
// SPDX-License-Identifier: GPL-3.0-only

#include <cstdlib>
#include <string>
#include <regex>
#include <iostream>
#include <fstream>
#include <sstream>

const std::string fileTemplate = R"(
struct HelpTxt
{
    const char *cmdstr;
    const char *helpstr;
};

const std::array<HelpTxt, ##ITEMS##> gs_helptxt = {
{)";


int main(int argc, char *argv[])
{
    std::cout << "Running doctool...\n";
    std::regex luaregex(R"(\/\/\/>\s(.*))");
    std::regex cmdregex(R"(([a-zA-Z0-9_]*))");

    if (argc<3)
    {
        std::cerr << "Usage: " << argv[0] << " <luacpp1 luacpp2 ..>  <output>\n";
        return EXIT_FAILURE;
    }

    std::string outputFileName(argv[argc-1]);    
    std::ofstream ofile(outputFileName);

    if (!ofile.good())
    {
        std::cerr << "Cannot open file " << outputFileName << "\n";
        return EXIT_FAILURE;
    }

    std::cout << "output: " << outputFileName << "\n";

    std::stringstream ss;
    ss << fileTemplate;
    bool firstItem = true;
    size_t items = 0;

    for(int arg=1; arg<(argc-1); arg++)
    {
        std::string inputFileName(argv[arg]);
        std::ifstream ifile(inputFileName);

        if (!ifile.good())
        {
            std::cerr << "Cannot open file " << inputFileName << "\n";
            return EXIT_FAILURE;
        }

        std::cout << "input : " << inputFileName << "\n";    
        
        for (std::string line; std::getline(ifile, line); ) 
        {
            std::smatch sm;
            if (std::regex_search(line, sm, luaregex))
            {
                auto helpStr = sm[1].str();

                std::smatch sm2;
                if (std::regex_search(helpStr, sm2, cmdregex))
                {
                    auto cmdStr = sm2[1];
                    if (!firstItem) 
                    {   
                        ss << ",";
                    }
                    ss << "{\"" << cmdStr << "\",\"" << helpStr << "\"}\n";
                    firstItem = false;
                    items++;
                }
            }
        }
    }

    ss << "}};\n\n";

    const std::string toReplace = "##ITEMS##";
    std::string txt = ss.str();
    size_t pos = txt.find(toReplace);
    txt.replace(pos, toReplace.length(), std::to_string(items));

    ofile << txt;

    std::cout << "produced " << items << " items\n\n";
    return EXIT_SUCCESS;
}
