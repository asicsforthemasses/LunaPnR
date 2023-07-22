/*  LunaPnR Source Code

    SPDX-License-Identifier: GPL-3.0-only
    SPDX-FileCopyrightText: 2022,2023 Niels Moseley <asicsforthemasses@gmail.com>
*/  

#include <cstdlib>
#include <sstream>
#include <fstream>
#include "lunacore.h"

int main(int argc, char *argv[])
{
    ChipDB::Design design;
    Scripting::Python p(&design);

    p.init();

    // in case of no arguments, give the user some idea what this is for.
    if (argc == 1)
    {
        std::cout << "--============================================================--\n";
        std::cout << "    LunaPnR Python subsystem test program\n";
        std::cout << "    Usage: core_python script1.py script2.py ..\n";
        std::cout << "--============================================================--\n\n";
        return EXIT_FAILURE;
    }

    // run each script in succession
    for(int arg=1; arg<argc; arg++)
    {
        std::string scriptFile = argv[arg];
        std::fstream ifile(scriptFile);
        if (!ifile.good())
        {
            std::cerr << "Cannot open script file " << scriptFile << "\n";
            return EXIT_FAILURE;
        }

        std::stringstream ss;
        ss << ifile.rdbuf();
        if (!p.executeScript(ss.str()))
        {
            std::cerr << "Script failed!\n";
            return EXIT_FAILURE;
        }
    }

    return EXIT_SUCCESS;
}
