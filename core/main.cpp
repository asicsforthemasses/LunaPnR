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

using namespace Logging;

int main(int argc, const char *argv[])
{
    setLogLevel(LogType::INFO);

    doLog(LogType::INFO, "\n");
	doLog(LogType::INFO, " /------------------------------------------------------------------------------\\\n");
	doLog(LogType::INFO, " |                                                                              |\n");
	doLog(LogType::INFO, " |  LunaPnR -- Luna Place and Route                                             |\n");
	doLog(LogType::INFO, " |                                                                              |\n");
	doLog(LogType::INFO, " |  Copyright (C) 2021 - 2024  Niels Moseley <asicsforthemasses@gmail.com>      |\n");
	doLog(LogType::INFO, " |                                                                              |\n");
	doLog(LogType::INFO, " |  Permission to use, copy, modify, and/or distribute this software for any    |\n");
	doLog(LogType::INFO, " |  purpose with or without fee is hereby granted, provided that the above      |\n");
	doLog(LogType::INFO, " |  copyright notice and this permission notice appear in all copies.           |\n");
	doLog(LogType::INFO, " |                                                                              |\n");
	doLog(LogType::INFO, " |  THE SOFTWARE IS PROVIDED \"AS IS\" AND THE AUTHOR DISCLAIMS ALL WARRANTIES    |\n");
	doLog(LogType::INFO, " |  WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF            |\n");
	doLog(LogType::INFO, " |  MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR     |\n");
	doLog(LogType::INFO, " |  ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES      |\n");
	doLog(LogType::INFO, " |  WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN       |\n");
	doLog(LogType::INFO, " |  ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF     |\n");
	doLog(LogType::INFO, " |  OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.              |\n");
	doLog(LogType::INFO, " |                                                                              |\n");
	doLog(LogType::INFO, " \\------------------------------------------------------------------------------/\n");
	doLog(LogType::INFO, "\n");

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
	doLog(LogType::INFO, ss.str());

	read_history(historyFile.c_str());

	//rl_bind_key('\t', rl_insert);
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
