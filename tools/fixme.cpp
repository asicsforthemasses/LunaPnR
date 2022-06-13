// SPDX-FileCopyrightText: 2021-2022 Niels Moseley, <n.a.moseley@moseleyinstruments.com>, et al.
// SPDX-FileCopyrightText: 2022 Niels Moseley <asicsforthemasses@gmail.com>
//
// SPDX-License-Identifier: GPL-3.0-only

#include <iostream>
#include <fstream>
#include <algorithm>
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

void process(const std::string filename, const std::string comment, size_t lineNumber, bool multiline)
{
    if (comment.find("FIXME") != std::string::npos)
    {
        std::cout << filename << "\n";
        if (multiline)
        {
            std::cout << "\tLine " << lineNumber << "\n";
            std::cout << red << comment << normal << "\n";
        }
        else
        {
            std::cout << "\tLine " << lineNumber << "  " << red << comment << normal << "\n";
        }
    }
    else if (comment.find("TODO") != std::string::npos)
    {
        std::cout << filename << "\n";
        if (multiline)
        {
            std::cout << "\tLine " << lineNumber << "\n";
            std::cout << yellow << comment << normal << "\n";
        }
        else
        {        
            std::cout << "\tLine " << lineNumber << "  " << yellow << comment << normal << "\n";
        }
    }
}

void scanFile(const std::string &filename)
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
                process(filename, comment, commentLine, false);
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
                process(filename, comment, commentLine, true);
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
}

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
        if (!dir_entry.is_directory())
        {
            auto path = dir_entry.path().string();
            if ((path.find(".cpp") != std::string::npos) || 
                (path.find(".h") != std::string::npos))
            {
                //std::cout << path << "\n";
                scanFile(path);
            }
        }
    }
}

