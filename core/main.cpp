// SPDX-FileCopyrightText: 2021-2024 Niels Moseley <asicsforthemasses@gmail.com>
//
// SPDX-License-Identifier: GPL-3.0-only

#include <cstdlib>
#include <iostream>
#include <filesystem>
#include <lunacore.h>

#ifdef USE_READLINE
    #include <readline/readline.h>
    #include <readline/history.h>
#endif

#define Q(x) #x
#define QUOTE(x) Q(x)

using namespace Logging;

std::string generateLine(const std::string &content)
{
    std::string result;
    result.append(" |  ");
    result.append(content);
    std::string spacer;
    spacer.assign(78 - result.length(),' ');
    result.append(spacer);
    result.append("|\n");
    return result;
}

int main(int argc, const char *argv[])
{
    setLogLevel(LogType::INFO);

    logInfo("\n");
    logInfo(" /----------------------------------------------------------------------------\\\n");
    logInfo(" |                                                                            |\n");
    logInfo(generateLine("LunaPnR " QUOTE(LUNAVERSIONSTRING) " -- Luna Place and Route"));
    logInfo(" |                                                                            |\n");
    logInfo(" |  Copyright (C) 2021 - 2024  Niels Moseley <asicsforthemasses@gmail.com>    |\n");
    logInfo(" |                                                                            |\n");
    logInfo(generateLine("Compiled on " __TIMESTAMP__));
    logInfo(" |                                                                            |\n");
    logInfo(" |  This program is free software: you can redistribute it and/or modify it   |\n");
    logInfo(" |  it under the terms of the GNU General Public License as published by      |\n");
    logInfo(" |  the Free Software Foundation, either version 3 of the License, or         |\n");
    logInfo(" |  (at your option) any later version.                                       |\n");
    logInfo(" |                                                                            |\n");
    logInfo(" |  This program is distributed in the hope that it will be useful,           |\n");
    logInfo(" |  but WITHOUT ANY WARRANTY; without even the implied warranty of            |\n");
    logInfo(" |  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the             |\n");
    logInfo(" |  GNU General Public License for more details.                              |\n");
    logInfo(" |                                                                            |\n");
    logInfo(" \\----------------------------------------------------------------------------/\n");
    logInfo("\n");

    LunaCore::Passes::registerAllPasses();

    LunaCore::Database db;

#ifdef USE_READLINE

    // See: https://eli.thegreenplace.net/2016/basics-of-using-the-readline-library/
    //      https://stackoverflow.com/questions/38792542/readline-h-history-usage-in-c

    std::filesystem::path lunaPnrDir;

    lunaPnrDir.append(getenv("HOME"));
    lunaPnrDir.append(".lunapnr");

    if (!std::filesystem::exists(lunaPnrDir))
    {
        std::filesystem::create_directory(lunaPnrDir);
    }

    std::filesystem::path historyFile = lunaPnrDir;
    historyFile.append(".history");

    // make special printable history file
    std::filesystem::path printableHistoryPath("~/");
    printableHistoryPath.append(".lunapnr/.history");

    std::stringstream ss;
    ss << "using history file: " << printableHistoryPath << "\n";
    logInfo(ss.str());

    read_history(historyFile.c_str());

    char* buf;
    bool exitLoop = false;

    while(!exitLoop && ((buf = readline("> ")) != nullptr))
    {
        if (strlen(buf) > 0)
        {
              add_history(buf);
        }

        if (strcmp(buf, "exit") != 0)
        {
            LunaCore::Passes::run(db, std::string(buf));
        }
        else
        {
            exitLoop = true;
        }

        // readline malloc's a new buffer every time.
        free(buf);
      }

    write_history(historyFile.c_str());
    history_truncate_file(historyFile.c_str(), 20);

#else
    std::string line;
    while(line != "exit")
    {
        std::cout << "> ";
        std::getline(std::cin, line);

        if (line != "exit")
        {
            LunaCore::Passes::run(db, line);
        }
    }
#endif

    return EXIT_SUCCESS;
}
