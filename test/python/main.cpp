#include <cstdlib>
#include <sstream>
#include <fstream>
#include "lunacore.h"


int main(int argc, char *argv[])
{
    ChipDB::Design design;    
    Scripting::Python p(&design);

    if (argc > 1)
    {
        //td::fstream ifile(argv[1], std::ios::binary);
        std::fstream ifile(argv[1]);
        if (!ifile.good())
        {
            std::cerr << "Cannot open script file " << argv[1] << "\n";
            return EXIT_FAILURE;
        }

        std::stringstream ss;
        ss << ifile.rdbuf();
        if (!p.executeScript(ss.str()))
        {
            std::cerr << "Script failed!\n";
            return EXIT_FAILURE;
        }
        return EXIT_SUCCESS;
    }
    else
    {
        if (!p.executeScript("import Luna\nprint('Hello from Python!')"))
        {            
            return EXIT_FAILURE;
        }
    }
    return EXIT_SUCCESS;
}
