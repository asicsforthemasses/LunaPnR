#include <cstdlib>
#include <regex>
#include <iostream>
#include <fstream>
#include <sstream>

int main(int argc, char *argv[])
{
    std::regex luaregex(R"(\/\/\/>\s(.*))");
    std::regex cmdregex(R"(([a-zA-Z0-9_]*))");

    if (argc<2)
    {
        std::cerr << "Usage: " << argv[0] << " <luacpp>\n";
        return EXIT_FAILURE;
    }

    std::ifstream ifile(argv[1]);
    bool first = true;
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
                if (!first) 
                {   
                    std::cout << ",";
                }
                std::cout << "{\"" << cmdStr << "\",\"" << helpStr << "\"}\n";
                first = false;
            }
        }
    }

    return EXIT_SUCCESS;
}
