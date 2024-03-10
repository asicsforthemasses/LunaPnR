// SPDX-FileCopyrightText: 2021-2024 Niels Moseley <asicsforthemasses@gmail.com>
//
// SPDX-License-Identifier: GPL-3.0-only

#include <stdexcept>
#include <sstream>
#include <array>

#include "defparser.h"
#include "common/logging.h"

using namespace ChipDB::DEF;

bool Parser::isWhitespace(char c)
{
    return ((c==' ') || (c == '\t'));
}

bool Parser::isAlpha(char c)
{
    if (((c >= 'A') && (c <= 'Z')) || ((c >= 'a') && (c <= 'z')))
        return true;


    if ((c == '_') || (c == '!'))
        return true;

    return false;
}

bool Parser::isDigit(char c)
{
    return ((c >= '0') && (c <= '9'));
}

bool Parser::isAlphaNumeric(char c)
{
    return (isAlpha(c) || isDigit(c));
}

bool Parser::isExtendedAlphaNumeric(char c)
{
    // DEF apparently allows '-' to be part of a string..
    return (isAlpha(c) || isDigit(c) || (c==']') || (c=='[')
        || (c=='-') || (c=='.') || (c=='_') || (c=='/')
        || (c=='\\') || (c=='<') || (c=='>'));
}

bool Parser::match(char c)
{
    if (peek() == c)
    {
        advance();
        return true;
    }
    return false;
}

void Parser::advance()
{
    if (!atEnd())
    {
        m_idx++;
        m_col++;
    }
}

char Parser::peek() const
{
    if (atEnd())
        return 0;

    return m_src->at(m_idx);
}

bool Parser::atEnd() const
{
    return !(m_idx < m_src->size());
}

#if 0
float Parser::toMicrons(const std::string &value)
{
    double v = 0.0;
    try
    {
        // *sigh* stod is locale dependent, so
        // it fucks up decimal point handling..
        setlocale(LC_ALL,"C");
        v = std::stod(value);
        setlocale (LC_ALL,"");
    }
    catch(const std::invalid_argument& ia)
    {
        doLog(LogType::ERROR,"Cannot convert %s into a float\n", value.c_str());
        error(ia.what());
    }

    return v;
}
#endif

int64_t Parser::flt2int(const std::string &value, bool &ok)
{
    const int32_t numberBase = 10;

    int64_t v = 0;
    ok = false;
    bool decimalPointSeen = false;
    uint32_t decimalDigits = 0;

    if (value.empty())
    {
        // error: to string, return 0
        return 0;
    }

    bool negative = false;
    for(uint32_t i=0; i<value.size(); i++)
    {
        char c = value[i];
        if ((i == 0) && (c == '-'))
        {
            negative = true;
        }
        else if (isDigit(c))
        {
            v *= numberBase;
            v += static_cast<int64_t>(c-'0');
            if (decimalPointSeen)
            {
                decimalDigits++;
            }
        }
        else if (c == '.')
        {
            decimalPointSeen = true;
        }
        else
        {
            // error.
            return 0;
        }
    }

    // check that we have 3 decimal digits
    // if not, adjust the value accordingly
    // so we get values in nanometers, no microns.

    //if ((decimalPointSeen) && (decimalDigits < 3))
    {
        while(decimalDigits < 3)
        {
            v *= numberBase;
            decimalDigits++;
        }
        while (decimalDigits > 3)
        {
            v /= numberBase;
            decimalDigits--;
        }
    }

    ok = true;
    return negative ? -v : v;
}

Parser::token_t Parser::tokenize(std::string &tokstr)
{
    const char NEWLINE = 10;
    const char CR = 13;

    tokstr.clear();

    // skip white space
    while(isWhitespace(peek()) && !atEnd())
    {
        advance();
    }

    if (atEnd())
    {
        return TOK_EOF;
    }

    // check for end-of-line
    if (match(NEWLINE))
    {
        m_lineNum++;
        m_col = 1;
        match(CR);
        return TOK_EOL;
    }

    if (match(CR))
    {
        m_lineNum++;
        m_col = 1;
        match(NEWLINE);
        return TOK_EOL;
    }

    // The hash is a line comment
    // so we read until the end of line
    // and emit an EOL token
    if (match('#'))
    {
        // check for end-of-line
        bool noEOL = true;
        while(noEOL)
        {
            if (match(NEWLINE))
            {
                m_lineNum++;
                m_col = 1;
                match(CR);
                noEOL = false;
            }
            else if (match(CR))
            {
                m_lineNum++;
                m_col = 1;
                match(NEWLINE);
                noEOL = false;
            }
            else
            {
                advance();
            }
        }
        //m_lineNum++;
        //m_col = 1;
        return TOK_EOL;
    }

    if (match(';'))
    {
        return TOK_SEMICOL;
    }

    if (match('('))
    {
        return TOK_LPAREN;
    }

    if (match(')'))
    {
        return TOK_RPAREN;
    }

    if (match('['))
    {
        return TOK_LBRACKET;
    }

    if (match(']'))
    {
        return TOK_RBRACKET;
    }

    if (match('+'))
    {
        return TOK_PLUS;
    }

    if (match('*'))
    {
        return TOK_STAR;
    }

    if (match('-'))
    {
        // could be the start of a number
        m_tokstr = "-";
        char c = peek();
        if (isDigit(c))
        {
            m_tokstr += c;
            advance();

            // it is indeed a number!
            c = peek();
            while(isDigit(c) || (c == '.') || (c == 'e'))
            {
                tokstr += c;
                advance();
                c = peek();
            }
            return TOK_NUMBER;
        }
        return TOK_MINUS;
    }

    // Identifiers
    if (isAlpha(peek()))
    {
        tokstr = peek();

        advance();
        char c = peek();
        while(isExtendedAlphaNumeric(c))
        {
            tokstr += c;
            advance();
            c = peek();
        }
        return TOK_IDENT;
    }

    // Quoted string
    if (match('"'))
    {
        // not sure what is legal but I've seen LEF files
        // with a newline in a string. For example, Google GF180MCU does this
        // in the tech lef file. 'PROPERTY LEF58_EOLENCLOSURE'
        //
        // the new strategy is to ignore newlines in a string
        // and keep on reading..

        char c = peek();
        //while((c != '"') && (c != NEWLINE) && (c != CR))
        while((c != '"'))
        {
            if ((c != NEWLINE) && (c != CR))
            {
                tokstr += c;
            }

            advance();
            c = peek();

            if (c == 0)
            {
                // unexpected EOF!
                error("Unexpected end of file. Missing closing \" in string?");
                return TOK_ERR;
            }
        }

        // skip closing quotes
        if (!match('"'))
        {
            return TOK_ERR;
        }

        return TOK_STRING;
    }

    // Number
    if (isDigit(peek()))
    {
        tokstr = peek();

        advance();
        char c = peek();

        //FIXME: overly relaxed float parsing.
        while(isDigit(c) || (c == '.') || (c == 'E') || (c == 'e') || (c == '+') || (c == '-'))
        {
            tokstr += c;
            advance();
            c = peek();
        }
        return TOK_NUMBER;
    }

    return TOK_ERR;
}

bool Parser::parse(const std::string &defstring)
{
    m_src = &defstring;
    m_idx = 0;
    m_col = 1;
    m_lineNum = 1;

    m_dBMicrons = 100.0;    // the default value mentioned in LEF/DEF documentation.

    m_tokstr.clear();
    bool m_inComment = false;

    m_curtok = TOK_EOF;
    do
    {
        m_curtok = tokenize(m_tokstr);
        if (m_curtok == TOK_ERR)
        {
            std::stringstream ss;
            ss << "Parse error, character = " << peek() << " dec: " << ((int)peek());
            error(ss.str());
            return false;
        }

        switch(m_curtok)
        {
        case TOK_HASH:  // line comment
            break;
        case TOK_IDENT:
            if (m_tokstr == "DESIGN")
            {
                if (!parseDesign()) return false;
            }
            else if (m_tokstr == "COMPONENTS")
            {
                if (!parseComponents()) return false;
            }
            else if (m_tokstr == "PROPERTYDEFINITIONS")
            {
                if (!parsePropertyDefinitions())
                {
                    error("Error parsing PROPERTYDEFINITIONS\n");
                    return false;
                }
            }
            else if (m_tokstr == "VIAS")
            {
                if (!parseVias())
                {
                    error("Error parsing VIAS\n");
                    return false;
                }
            }
            else if (m_tokstr == "END")
            {
                // eat everything until EOL
                // so we don't get in a weird state.
                skipUntilEOL();
            }
            else if (!m_tokstr.empty())
            {
                Logging::doLog(Logging::LogType::VERBOSE, "  DEF skipping: %s on line %u\n", m_tokstr.c_str(), m_lineNum);
            }
            break;
        default:
            if (!m_tokstr.empty())
            {
                Logging::doLog(Logging::LogType::VERBOSE, "  DEF skipping: %s on line %u\n", m_tokstr.c_str(), m_lineNum);
            }
            break;
        }
    } while(m_curtok != TOK_EOF);

    onEndParse();

    return true;
}

void Parser::error(const char *errstr) const
{
    std::stringstream ss;
    ss << "Line " << m_lineNum << " col " << m_col << " : " << errstr << "\n";
    Logging::doLog(Logging::LogType::ERROR, ss.str());
    throw std::runtime_error(ss.str());
}

void Parser::error(const std::string &errstr) const
{
    std::stringstream ss;
    ss << "Line " << m_lineNum << " col " << m_col << " : " << errstr << "\n";
    Logging::doLog(Logging::LogType::ERROR, ss.str());
    throw std::runtime_error(ss.str());
}

bool Parser::parseDesign()
{
    std::string name;

    // design name
    m_curtok = tokenize(name);
    if (m_curtok != TOK_IDENT)
    {
        error("Expected a design name");
        return false;
    }

    // expect SEMICOL
    m_curtok = tokenize(m_tokstr);
    if (m_curtok != TOK_SEMICOL)
    {
        error("Expected ;\n");
        return false;
    }

    // expect EOL
    m_curtok = tokenize(m_tokstr);
    if (m_curtok != TOK_EOL)
    {
        error("Expected EOL\n");
        return false;
    }

    onDesign(name);

    return true;
}

#if 0
bool Parser::parseEnd()
{
    m_curtok = tokenize(m_tokstr);
    if (m_tokstr != "DESIGN")
    {
        error("Expected DESIGN\n");
        return false;
    }

    std::string designName;
    m_curtok = tokenize(designName);
    if (m_curtok != TOK_IDENT)
    {
        error("Expected END DESIGN name ;\n");
        return false;
    }

    m_curtok = tokenize(m_tokstr);
    if (m_curtok != TOK_SEMICOL)
    {
        error("Expected ;\n");
        return false;
    }

    onEndDesign(designName);

    return true;
}
#endif

bool Parser::parseComponents()
{
    std::string numCompString;
    m_curtok = tokenize(numCompString);
    if (m_curtok != TOK_NUMBER)
    {
        error("Expected a number after COMPONENTS");
        return false;
    }

    m_curtok = tokenize(m_tokstr);
    if (m_curtok != TOK_SEMICOL)
    {
        error("Expected ; after COMPONENTS <number>");
        return false;
    }

    m_curtok = tokenize(m_tokstr);
    if (m_curtok != TOK_EOL)
    {
        error("Expected EOL after COMPONENTS <number> ;");
        return false;
    }

    int numComponents = 0;
    try
    {
        numComponents = std::stoul(numCompString);
    }
    catch(std::invalid_argument &)
    {
        std::stringstream ss;
        ss << "Invalid number of components: " << numComponents;
        error(ss.str());
        return false;
    }

    // let's not trust the number of components..
    // but rely on the '-' prefix

    m_curtok = tokenize(m_tokstr);
    while(m_curtok == TOK_MINUS)
    {
        if (!parseComponent())
        {
            return false;
        }
        m_curtok = tokenize(m_tokstr);  // next minus
    }

    if ((m_curtok != TOK_IDENT) || (m_tokstr != "END"))
    {
        error("Expected END");
        return false;
    }

    m_curtok = tokenize(m_tokstr);
    if ((m_curtok != TOK_IDENT) || (m_tokstr != "COMPONENTS"))
    {
        error("Expected COMPONENTS after END");
        return false;
    }

    m_curtok = tokenize(m_tokstr);
    if (m_curtok != TOK_EOL)
    {
        error("Expected EOL after END COMPONENTS");
        return false;
    }

    return true;
}

bool Parser::parseComponent()
{
    std::string insName;
    m_curtok = tokenize(insName);
    if (m_curtok != TOK_IDENT)
    {
        error("Expected a component/instance name");
        return false;
    }

    std::string archetype;
    m_curtok = tokenize(archetype);
    if (m_curtok != TOK_IDENT)
    {
        error("Expected a component macro/cell name");
        return false;
    }

    m_curtok = tokenize(m_tokstr);
    if ((m_curtok != TOK_EOL) && (m_curtok != TOK_SEMICOL))
    {
        error("Expected EOL or ;");
        return false;
    }

    onComponent(insName, archetype);

    // the component does not have any further
    // information -> exit.
    if (m_curtok == TOK_SEMICOL)
    {
        m_curtok = tokenize(m_tokstr);
        if (m_curtok != TOK_EOL)
        {
            error("Expected EOL");
            return false;
        }
        return true;
    }

    // read the first '+'
    m_curtok = tokenize(m_tokstr);
    while (m_curtok == TOK_PLUS)
    {
        // get next ident after the '+'.
        m_curtok = tokenize(m_tokstr);
        if (m_curtok == TOK_IDENT)
        {
            if (m_tokstr == "FIXED")
            {
                if (!parseFixed()) return false;
                m_curtok = tokenize(m_tokstr);
            }
            else if (m_tokstr == "PLACED")
            {
                if (!parsePlaced()) return false;
                m_curtok = tokenize(m_tokstr);
            }
            else if (m_tokstr == "UNPLACED")
            {
                if (!parseUnplaced()) return false;
                m_curtok = tokenize(m_tokstr);
            }
            else
            {
                // unknown statement ..
                skipUntilEOLorSemicolon();
            }

            // check if we have an EOL
            // if we do, it means there
            // should be more to come
            // but if we get a ';'
            // this is the end.. my friend.
            if (m_curtok == TOK_EOL)
            {
                // read in the next '+'.
                m_curtok = tokenize(m_tokstr);
            }
        }
    }

    if (m_curtok != TOK_SEMICOL)
    {
        error("Expected ; at end of component");
        return false;
    }

    m_curtok = tokenize(m_tokstr);

    if (m_curtok != TOK_EOL)
    {
        error("Expected EOL at end of component");
        return false;
    }

    return true;
}

bool Parser::parsePlaced()
{
    auto point = parsePoint();
    if (!point) return false;

    std::string orient;
    m_curtok = tokenize(orient);
    if (m_curtok != TOK_IDENT)
    {
        error("Expected an orientation string");
        return false;
    }

    auto const pl = ChipDB::PlacementInfo{ChipDB::PlacementInfo::PLACED};
    if (orient == "N")
    {
        onComponentPlacement(point.value(), pl, ChipDB::Orientation{ChipDB::Orientation::R0});
    }
    else if (orient == "S")
    {
        onComponentPlacement(point.value(), pl, ChipDB::Orientation{ChipDB::Orientation::R180});
    }
    else if (orient == "W")
    {
        onComponentPlacement(point.value(), pl, ChipDB::Orientation{ChipDB::Orientation::R90});
    }
    else if (orient == "E")
    {
        onComponentPlacement(point.value(), pl, ChipDB::Orientation{ChipDB::Orientation::R270});
    }
    else if (orient == "FN")
    {
        onComponentPlacement(point.value(), pl, ChipDB::Orientation{ChipDB::Orientation::MY});
    }
    else if (orient == "FS")
    {
        onComponentPlacement(point.value(), pl, ChipDB::Orientation{ChipDB::Orientation::MX});
    }
    else if (orient == "FW")
    {
        onComponentPlacement(point.value(), pl, ChipDB::Orientation{ChipDB::Orientation::MX90});
    }
    else if (orient == "FE")
    {
        onComponentPlacement(point.value(), pl, ChipDB::Orientation{ChipDB::Orientation::MY90});
    }
    else
    {
        std::stringstream ss;
        ss << "Unrecognized orientation: " << orient;
        error(ss.str());
        return false;
    }

    return true;
}

bool Parser::parseUnplaced()
{
    onComponentPlacement({0,0}, ChipDB::PlacementInfo{ChipDB::PlacementInfo::UNPLACED},
        ChipDB::Orientation{ChipDB::Orientation::UNDEFINED});

    return true;
}

bool Parser::parseFixed()
{
    auto point = parsePoint();
    if (!point) return false;

    std::string orient;
    m_curtok = tokenize(orient);
    if (m_curtok != TOK_IDENT)
    {
        error("Expected an orientation string");
        return false;
    }

    auto const pl = ChipDB::PlacementInfo{ChipDB::PlacementInfo::PLACEDANDFIXED};
    if (orient == "N")
    {
        onComponentPlacement(point.value(), pl, ChipDB::Orientation{ChipDB::Orientation::R0});
    }
    else if (orient == "S")
    {
        onComponentPlacement(point.value(), pl, ChipDB::Orientation{ChipDB::Orientation::R180});
    }
    else if (orient == "W")
    {
        onComponentPlacement(point.value(), pl, ChipDB::Orientation{ChipDB::Orientation::R90});
    }
    else if (orient == "E")
    {
        onComponentPlacement(point.value(), pl, ChipDB::Orientation{ChipDB::Orientation::R270});
    }
    else if (orient == "FN")
    {
        onComponentPlacement(point.value(), pl, ChipDB::Orientation{ChipDB::Orientation::MY});
    }
    else if (orient == "FS")
    {
        onComponentPlacement(point.value(), pl, ChipDB::Orientation{ChipDB::Orientation::MX});
    }
    else if (orient == "FW")
    {
        onComponentPlacement(point.value(), pl, ChipDB::Orientation{ChipDB::Orientation::MX90});
    }
    else if (orient == "FE")
    {
        onComponentPlacement(point.value(), pl, ChipDB::Orientation{ChipDB::Orientation::MY90});
    }
    else
    {
        std::stringstream ss;
        ss << "Unrecognized orientation: " << orient;
        error(ss.str());
        return false;
    }

    return true;
}

std::optional<ChipDB::Coord64> Parser::parsePoint()
{
    // A point can be ( <int> <int> )
    // and any of the <int> can be '*'
    // which means: same as previous point.

    m_curtok = tokenize(m_tokstr);
    if (m_curtok != TOK_LPAREN)
    {
        error("Expected ( in position/point\n");
        return std::nullopt;
    }

    ChipDB::Coord64 pos;

    std::string xNumStr;
    m_curtok = tokenize(xNumStr);
    if ((m_curtok != TOK_NUMBER) && (m_curtok != TOK_STAR))
    {
        error("Expected number for x coordinate in position/point");
        return std::nullopt;
    }

    std::string yNumStr;
    m_curtok = tokenize(yNumStr);
    if ((m_curtok != TOK_NUMBER) && (m_curtok != TOK_STAR))
    {
        error("Expected number for y coordinate in position/point");
        return std::nullopt;
    }

    m_curtok = tokenize(m_tokstr);
    if (m_curtok != TOK_RPAREN)
    {
        error("Expected ) in position/point");
        return std::nullopt;
    }

    if (xNumStr == "*")
    {
        pos.m_x = m_lastPoint.m_x;
    }
    else
    {
        try
        {
            pos.m_x = std::stold(xNumStr);
        }
        catch(const std::invalid_argument&)
        {
            std::stringstream ss;
            ss << "Point has invalid number: " << xNumStr;
            error(ss.str());
            return std::nullopt;
        }
    }

    if (yNumStr == "*")
    {
        pos.m_y = m_lastPoint.m_y;
    }
    else
    {
        try
        {
            pos.m_y = std::stold(yNumStr);
        }
        catch(const std::invalid_argument&)
        {
            std::stringstream ss;
            ss << "Point has invalid number: " << yNumStr;
            error(ss.str());
            return std::nullopt;
        }
    }

    m_lastPoint = pos;

    return pos;
}

bool Parser::skipUntilSemicolon()
{
    std::string name;
    m_curtok = tokenize(name);
    while(m_curtok != TOK_SEMICOL)
    {
        if (atEnd())
        {
            error("Unexpected end of file");
            return false;
        }
        m_curtok = tokenize(name);
    }

    return true;
}

bool Parser::skipUntilEOL()
{
    std::string name;
    m_curtok = tokenize(name);
    while(m_curtok != TOK_EOL)
    {
        if (atEnd())
        {
            error("Unexpected end of file");
            return false;
        }
        m_curtok = tokenize(name);
    }

    return true;
}

bool Parser::skipUntilEOLorSemicolon()
{
    std::string name;
    m_curtok = tokenize(name);
    while((m_curtok != TOK_EOL) && (m_curtok != TOK_SEMICOL))
    {
        if (atEnd())
        {
            error("Unexpected end of file\n");
            return false;
        }
        m_curtok = tokenize(name);
    }

    return true;
}

bool Parser::parseUntilEnd(const std::string &postfix)
{
    bool exit = false;
    while(!exit)
    {
        if (atEnd()) return false;

        m_curtok = tokenize(m_tokstr);
        if (!m_tokstr.empty())
        {
            Logging::doLog(Logging::LogType::VERBOSE,"  DEF skipping: %s on line %d\n", m_tokstr.c_str(), m_lineNum);
        }

        if (m_curtok == TOK_IDENT)
        {
            if (m_tokstr == "END")
            {
                m_curtok = tokenize(m_tokstr);
                if (m_tokstr == postfix)
                {
                    exit = true;
                }
            }
            else
            {
                if (!m_tokstr.empty())
                {
                    Logging::doLog(Logging::LogType::VERBOSE,"  DEF skipping: %s on line %d\n", m_tokstr.c_str(), m_lineNum);
                }
            }
        }
    }

    m_curtok = tokenize(m_tokstr);
    if (m_curtok != TOK_EOL)
    {
        std::stringstream ss;
        ss << "Expected EOL after END ";
        ss << postfix << "\n";
        error(ss.str());
        return false;
    }

    return true;
}

bool Parser::parsePropertyDefinitions()
{
    // we don't do anything with this yet
    // skip until we get END PROPERTYDEFINITIONS <eol>

    return parseUntilEnd("PROPERTYDEFINITIONS");
}


bool Parser::parseVias()
{
    // we don't do anything with this yet
    // skip until we get END VIAS <eol>

    return parseUntilEnd("VIAS");
}
