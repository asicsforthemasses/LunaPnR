// SPDX-FileCopyrightText: 2021-2025 Niels Moseley <asicsforthemasses@gmail.com>
//
// SPDX-License-Identifier: GPL-3.0-only

#include <iostream>
#include <fstream>
#include <algorithm>
#include <array>
#include <vector>
#include <filesystem>

enum class State
{
    IDLE = 0,
    SLASH,
    LINECOMMENT,
    MULTILINE,
    MULTILINE2
};

/*

         foreground background
black        30         40
red          31         41
green        32         42
yellow       33         43
blue         34         44
magenta      35         45
cyan         36         46
white        37         47

Additionally, you can use these:

reset             0  (everything back to normal)
bold/bright       1  (often a brighter shade of the same colour)
underline         4
inverse           7  (swap foreground and background colours)
bold/bright off  21
underline off    24
inverse off      27

*/

constexpr const char red[] = "\033[1;31m";
constexpr const char yellow[] = "\033[1;33m";
constexpr const char normal[] = "\033[0m\n";

bool hasSPDX(const std::string comment)
{
    if (comment.find("SPDX") == std::string::npos)
    {
        return false;
    }
    return true;
}

bool hasCC(const std::string comment)
{
    if (comment.find("Copyright") == std::string::npos)
    {
        return false;
    }
    return true;
}

bool scanFile(const std::string &filename)
{
    size_t lineNumber = 1;
    State state = State::IDLE;

    std::string contents;

    size_t      commentLine;
    std::string comment;

    {
        std::ifstream srcFile(filename, std::ios::binary);
        std::stringstream buffer;
        buffer << srcFile.rdbuf();
        contents = buffer.str();
    }

    bool needsSPDX = false;
    bool hasCopyright = false;

    size_t idx = 0;
    while(idx < contents.size())
    {
        char c = contents.at(idx);
        switch(state)
        {
        case State::IDLE:
            if (c == '\n')
            {
                lineNumber++;
            }
            else if (c == '/')
            {
                comment = c;
                state = State::SLASH;
            }
            idx++;
            break;
        case State::SLASH:
            if (c == '/')
            {
                comment += c;
                state = State::LINECOMMENT;
                commentLine = lineNumber;
            }
            else if (c == '*')
            {
                comment += c;
                state = State::MULTILINE;
                commentLine = lineNumber;
            }
            else
            {
                state = State::IDLE;
            }
            idx++;
            break;
        case State::LINECOMMENT:
            if ((c == '\n') || (c == '\r'))
            {
                lineNumber++;
                state = State::IDLE;
                if (!hasSPDX(comment))
                {
                    needsSPDX = true;
                }
                if (hasCC(comment))
                {
                    hasCopyright = true;
                }
            }
            else
            {
                comment += c;
            }
            idx++;
            break;
        case State::MULTILINE:
            if (c == '\n')
            {
                lineNumber++;
            }
            else if (c == '*')
            {
                state = State::MULTILINE2;
            }
            comment += c;
            idx++;
            break;
        case State::MULTILINE2:
            if (c == '/')
            {
                state = State::IDLE;
                if (!hasSPDX(comment))
                {
                    needsSPDX = true;
                }

                if (hasCC(comment))
                {
                    hasCopyright = true;
                }
            }
            else if (c == '*')
            {
                state = State::MULTILINE2;
            }
            else if (c == '\n')
            {
                lineNumber++;
                state = State::MULTILINE;
            }
            else
            {
                state = State::MULTILINE;
            }
            comment += c;
            idx++;
            break;
        }
    }

    if (needsSPDX)
    {
        std::cout << "  " << filename << " needs a SPDX header\n";
    }

    if (hasCopyright)
    {
        std::cout << "  " << filename << " has an old copyright header\n";
    }

    return needsSPDX;
}

bool inSkipList(const std::string &path)
{
    std::array<std::string, 8> list = {
        ".vscode", "cmake", "doc", "examples", "resources", "scripts", "contrib", "build"
    };

    for(auto item : list)
    {
        if (path.find(item) != std::string::npos)
        {
            return true;
        }
    }

    return false;
}

const char spdxHeader[] = "/*\n" \
"  LunaPnR Source Code\n" \
"  \n" \
"  SPDX-License-Identifier: GPL-3.0-only\n" \
"  SPDX-FileCopyrightText: 2022,2023 Niels Moseley <asicsforthemasses@gmail.com>\n" \
"*/\n\n";

int main(int argc, char *argv[])
{
    std::string rootpath = ".";

    if (argc > 1)
    {
        rootpath = argv[1];
    }

    for (const std::filesystem::directory_entry& dir_entry :
        std::filesystem::recursive_directory_iterator{rootpath})
    {
        auto path = dir_entry.path().string();

        if (inSkipList(path))
        {
            continue;
        }

        if (!dir_entry.is_directory())
        {
            if (path.find(".html") != std::string::npos)
            {
                continue;
            }

            if ((path.find(".cpp") != std::string::npos) ||
                (path.find(".h") != std::string::npos))
            {
                if (scanFile(path))
                {
                    // patch file
                    std::ifstream srcFileIn(path, std::ios::binary);
                    std::stringstream buffer;
                    buffer << srcFileIn.rdbuf();
                    std::string contents = buffer.str();
                    srcFileIn.close();

                    std::ofstream srcFileOut(path, std::ios::binary);
                    srcFileOut << spdxHeader << "\n";
                    srcFileOut << contents;
                    srcFileOut.close();
                }
            }
        }
    }
}

