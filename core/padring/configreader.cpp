// SPDX-FileCopyrightText: 2021-2024 Niels Moseley <asicsforthemasses@gmail.com>
//
// SPDX-License-Identifier: GPL-3.0-only

#include <sstream>
#include <algorithm>
#include <cstdarg>
#include "common/logging.h"
#include "configreader.hpp"

namespace LunaCore::Padring
{

void ConfigReader::error(const std::string_view fmt, ...)
{
    std::stringstream ss;
    ss << "Line: " << m_lineNum << " : " << fmt;

    std::va_list args;
    va_start(args, fmt);
    Logging::logError(ss.str(), args);
    va_end(args);
}

void ConfigReader::warning(const std::string_view fmt, ...)
{
    std::stringstream ss;
    ss << "Line: " << m_lineNum << " : " << fmt;

    std::va_list args;
    va_start(args, fmt);
    Logging::logWarning(ss.str(), args);
    va_end(args);
}

void ConfigReader::info(const std::string_view fmt, ...)
{
    std::stringstream ss;
    ss << "Line: " << m_lineNum << " : " << fmt;

    std::va_list args;
    va_start(args, fmt);
    Logging::logInfo(ss.str(), args);
    va_end(args);
}

ConfigReader::Token_t ConfigReader::tokenize(std::string &tokstr)
{
    tokstr.clear();

    while(isWhitespace(m_tokchar) && !m_is.eof())
    {
        m_tokchar = m_is.get();
    }

    if (m_is.eof())
    {
        return Token_t::END;
    }

    if ((m_tokchar==10) || (m_tokchar==13))
    {
        m_tokchar = m_is.get();
        m_lineNum++;
        return Token_t::EOL;
    }

    if (match('#'))
    {
        return Token_t::HASH;
    }

    if (match(';'))
    {
        return Token_t::SEMICOL;
    }

    if (match('('))
    {
        return Token_t::LPAREN;
    }

    if (match(')'))
    {
        return Token_t::RPAREN;
    }

    if (match('['))
    {
        return Token_t::LBRACKET;
    }

    if (match(']'))
    {
        return Token_t::RBRACKET;
    }

    if (match('-'))
    {
        // could be the start of a number
        tokstr = m_tokchar;
        m_tokchar = m_is.get();
        if (isDigit(m_tokchar))
        {
            // it is indeed a number!
            while(isDigit(m_tokchar) || (m_tokchar == '.') || (m_tokchar == 'e'))
            {
                tokstr += m_tokchar;
                m_tokchar = m_is.get();
            }
            return Token_t::NUMBER;
        }
        return Token_t::MINUS;
    }

    if (isAlpha(m_tokchar))
    {
        tokstr = m_tokchar;
        m_tokchar = m_is.get();
        while(isAlphaNumeric(m_tokchar) || isSpecialIdentChar(m_tokchar))
        {
            tokstr += m_tokchar;
            m_tokchar = m_is.get();
        }
        return Token_t::IDENT;
    }

    if (match('"'))
    {
        m_tokchar = m_is.get();
        while((m_tokchar != '"') && (m_tokchar != 10) && (m_tokchar != 13))
        {
            tokstr += m_tokchar;
            m_tokchar = m_is.get();
        }

        // skip closing quotes
        if (match('"'))
        {
        }

        // error on newline
        if ((m_tokchar == 10) || (m_tokchar == 13))
        {
            // TODO: error, string cannot continue after newline!
        }
        return Token_t::STRING;
    }

    if (isDigit(m_tokchar))
    {
        tokstr = m_tokchar;
        m_tokchar = m_is.get();
        while(isDigit(m_tokchar) || (m_tokchar == '.') || (m_tokchar == 'e'))
        {
            tokstr += m_tokchar;
            m_tokchar = m_is.get();
        }
        return Token_t::NUMBER;
    }

    m_tokchar = m_is.get();
    return Token_t::ERR;
}

bool ConfigReader::parse()
{
    m_lineNum = 1;

    if (!m_is.good())
    {
        error("ConfigReader: input stream is not open\n");
        return false;
    }

    std::string tokstr;
    m_tokchar = m_is.get();

    bool m_inComment = false;

    ConfigReader::Token_t tok = Token_t::END;
    do
    {
        tok = tokenize(tokstr);
        if (!m_inComment)
        {
            switch(tok)
            {
            case Token_t::ERR:
                error("Config parse error\n");
                break;
            case Token_t::HASH:  // line comment
                m_inComment = true;
                break;
            case Token_t::IDENT:
                if (tokstr == "CORNER")
                {
                    if (!parseCorner())
                        return false;
                }
                else if (tokstr == "PAD")
                {
                    if (!parsePad())
                        return false;
                }
                else if (tokstr == "SPACE")
                {
                    if (!parseSpace())
                        return false;
                }
                else if (tokstr == "FILLER")
                {
                    if (!parseFiller())
                        return false;
                }
                else if (tokstr == "OFFSET")
                {
                    if (!parseOffset())
                        return false;
                }
                else
                {
                    std::stringstream ss;
                    ss << "unrecognized item " << tokstr << "\n";
                    error(ss.str());
                }
                break;
            default:
                ;
            }
        }
        else
        {
            if (tok == Token_t::EOL)
            {
                m_inComment = false;
            }
        }
    } while(tok != Token_t::END);

    return true;
}

bool ConfigReader::parsePad()
{
    // PAD: instance location cellname
    std::string tokstr;
    std::string instance;
    std::string location;
    bool flipped = false;

    // instance name
    auto tok = tokenize(instance);
    if (tok != Token_t::IDENT)
    {
        error("Expected an instance name\n");
        return false;
    }

    // location name
    tok = tokenize(location);
    if (tok != Token_t::IDENT)
    {
        error("Expected a location\n");
        return false;
    }

    // PADs can only be on North, South, East or West
    std::array<std::string, 4> items = {"N","E","S","W"};
    if (!inArray(location, items))
    {
        error("Expected a pad location to be one of N/E/S/W\n");
        return false;
    }

#if 0
    // parse optional 'FLIP' argument for flipped cells
    tok = tokenize(cellname);
    if ((tok == Token_t::IDENT) && (cellname == "FLIP"))
    {
        flipped = true;
        tok = tokenize(cellname);
    }
#endif

    // expect semicol
    tok = tokenize(tokstr);
    if (tok != Token_t::SEMICOL)
    {
        error("Expected ;\n");
        return false;
    }

    m_padCount++;
    onPad(instance,location,flipped);

    return true;
}

bool ConfigReader::inArray(const std::string &value, std::span<std::string> span)
{
    return std::find(span.begin(), span.end(), value) != span.end();
}

bool ConfigReader::parseCorner()
{
    // CORNER: instance location
    std::string tokstr;
    std::string instance;
    std::string location;

    // instance name
    auto tok = tokenize(instance);
    if (tok != Token_t::IDENT)
    {
        error("Expected an instance name\n");
        return false;
    }

    // location name
    tok = tokenize(location);
    if (tok != Token_t::IDENT)
    {
        error("Expected a location\n");
        return false;
    }

    // corners can only be on NorthWest, SouthWest, SouthEast or NorthEast
    std::array<std::string, 4> items = {"NW","SW","SE","NE"};
    if (!inArray(location, items))
    {
        error("Expected a corner location to be one of NW/SW/SE/NE\n");
        return false;
    }

    // expect semicol
    tok = tokenize(tokstr);
    if (tok != Token_t::SEMICOL)
    {
        error("Expected ;\n");
        return false;
    }

    onCorner(instance,location);
    return true;
}


bool ConfigReader::parseSpace()
{
    // SPACE: g
    std::string tokstr;
    std::string g;

    // space
    auto tok = tokenize(g);
    if (tok != Token_t::NUMBER)
    {
        error("Expected a number for space\n");
        return false;
    }

    // expect semicol
    tok = tokenize(tokstr);
    if (tok != Token_t::SEMICOL)
    {
        error("Expected ;\n");
        return false;
    }

    double gd;
    try
    {
        gd = std::stod(g);
    }
    catch(const std::invalid_argument& ia)
    {
        error(ia.what());
    }

    onSpace(gd);
    return true;
}


bool ConfigReader::parseOffset()
{
    // OFFSET: g
    std::string tokstr;
    std::string g;

    // offset
    auto tok = tokenize(g);
    if (tok != Token_t::NUMBER)
    {
        error("Expected a number for offset\n");
        return false;
    }

    // expect semicol
    tok = tokenize(tokstr);
    if (tok != Token_t::SEMICOL)
    {
        error("Expected ;\n");
        return false;
    }

    double gd;
    try
    {
        gd = std::stod(g);
    }
    catch(const std::invalid_argument& ia)
    {
        error(ia.what());
    }

    onOffset(gd);
    return true;
}

bool ConfigReader::parseFiller()
{
    // FILLER: fillername
    std::string tokstr;
    std::string fillerName;

    // fillername
    auto tok = tokenize(fillerName);
    if (tok != Token_t::IDENT)
    {
        error("Expected a filler cell prefix\n");
        return false;
    }

    // expect semicol
    tok = tokenize(tokstr);
    if (tok != Token_t::SEMICOL)
    {
        error("Expected ;\n");
        return false;
    }

    onFiller(fillerName);
    return true;
}

};
