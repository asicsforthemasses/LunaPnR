#include <cstdlib>
#include <string>
#include <regex>
#include <iostream>
#include <fstream>
#include <sstream>

/* TODO: arse 

  shit
*/

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
        std::cerr << "Usage: " << argv[0] << " <luacpp> <output>\n";
        return EXIT_FAILURE;
    }

    std::ifstream ifile(argv[1]);
    std::ofstream ofile(argv[2]);

    std::cout << "input : " << argv[1] << "\n";
    std::cout << "output: " << argv[2] << "\n";

    std::stringstream ss;

    ss << fileTemplate;

    bool first = true;
    size_t items = 0;
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
                    ss << ",";
                }
                ss << "{\"" << cmdStr << "\",\"" << helpStr << "\"}\n";
                first = false;
                items++;
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
