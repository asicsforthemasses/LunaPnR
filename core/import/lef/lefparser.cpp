// SPDX-FileCopyrightText: 2021-2024 Niels Moseley <asicsforthemasses@gmail.com>
//
// SPDX-License-Identifier: GPL-3.0-only

#include <stdexcept>
#include <sstream>
#include <array>

#include "lefparser.h"
#include "common/logging.h"

using namespace ChipDB::LEF;

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
    // LEF apparently allows '-' to be part of a string..
    return (isAlpha(c) || isDigit(c) || (c==']') || (c=='[') || (c=='-'));
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
                error("Unexpected end of file. Missing closing \" in string?\n");
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

bool Parser::parse(const std::string &lefstring)
{
    m_src = &lefstring;
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
            Logging::doLog(Logging::LogType::ERROR, "LEF parse error, current character = '%c' (0x%02X)\n", peek(), static_cast<uint32_t>(peek()));
            error("");
            return false;
        }

        switch(m_curtok)
        {
        case TOK_HASH:  // line comment
            break;
        case TOK_IDENT:

            // make sure the token string is upper case
            // FIXME: do this for all the other keywords
            std::transform(m_tokstr.begin(), m_tokstr.end(), m_tokstr.begin(),
                   [](unsigned char c){ return std::toupper(c); }
            );

            if (m_tokstr == "MACRO")
            {
                parseMacro();
            }
            else if (m_tokstr == "LAYER")
            {
                parseLayer();
            }
            else if (m_tokstr == "VIA")
            {
                parseVia();
            }
            else if (m_tokstr == "VIARULE")
            {
                parseViaRule();
            }
            else if (m_tokstr == "UNITS")
            {
                parseUnits();
            }
            else if (m_tokstr == "PROPERTYDEFINITIONS")
            {
                parsePropertyDefintions();
            }
            else if (m_tokstr == "MANUFACTURINGGRID")
            {
                parseManufacturingGrid();
            }
            else if (m_tokstr == "SITE")
            {
                parseSite();
            }
            else
            {
                if (!m_tokstr.empty())
                {
                    Logging::doLog(Logging::LogType::VERBOSE, "  LEF skipping: %s on line %u\n", m_tokstr.c_str(), m_lineNum);
                }
            }
            break;
        default:
            if (!m_tokstr.empty())
            {
                Logging::doLog(Logging::LogType::VERBOSE, "  LEF skipping: %s on line %u\n", m_tokstr.c_str(), m_lineNum);
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

bool Parser::skipUntilSemicolon()
{
    std::string name;
    m_curtok = tokenize(name);
    while(m_curtok != TOK_SEMICOL)
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

bool Parser::parseMacro()
{
    std::string name;


    // macro name
    m_curtok = tokenize(name);
    if (m_curtok != TOK_IDENT)
    {
        error("Expected a macro name\n");
        return false;
    }

    //std::cout << "MACRO " << name << "\n";

    // expect EOL
    m_curtok = tokenize(m_tokstr);
    if (m_curtok != TOK_EOL)
    {
        error("Expected EOL\n");
        return false;
    }

    onMacro(name);

    // wait for 'END macroname'
    bool endFound = false;
    while(true)
    {
        m_curtok = tokenize(m_tokstr);

        if (m_curtok == TOK_IDENT)
        {
            if (m_tokstr == "PIN")
            {
                parsePin();
            }
            else if (m_tokstr == "CLASS")
            {
                parseClass();
            }
            else if (m_tokstr == "ORIGIN")
            {
                parseOrigin();
            }
            else if (m_tokstr == "FOREIGN")
            {
                parseForeign();
            }
            else if (m_tokstr == "SIZE")
            {
                parseSize();
            }
            else if (m_tokstr == "SYMMETRY")
            {
                parseSymmetry();
            }
            else if (m_tokstr == "SITE")
            {
                parseMacroSite();
            }
            else if (m_tokstr == "OBS")
            {
                parseObstruction();
            }
            //else if (m_tokstr == "LAYER")
            //{
            //    parseLayer();   // TECH LEF layer, not a port LAYER!
            //}
        }

        if (endFound)
        {
            if ((m_curtok == TOK_IDENT) && (m_tokstr == name))
            {
                onEndMacro(name);
                return true;
            }
        }
        else if ((m_curtok == TOK_IDENT) && (m_tokstr == "END"))
        {
            endFound = true;
        }
        else
        {
            endFound = false;
        }

        if (atEnd())
        {
            error("Unexpected end of file\n");
            return false;
        }
    }
}

bool Parser::parseSite()
{
    std::string name;

    // site name
    m_curtok = tokenize(name);
    if (m_curtok != TOK_IDENT)
    {
        error("Expected a SITE name\n");
        return false;
    }

    // expect EOL
    m_curtok = tokenize(m_tokstr);
    if (m_curtok != TOK_EOL)
    {
        error("Expected EOL\n");
        return false;
    }

    onSite(name);

    // wait for 'END site name'
    bool endFound = false;
    while(true)
    {
        m_curtok = tokenize(m_tokstr);

        if (m_curtok == TOK_IDENT)
        {
            bool ok = false;
            if (m_tokstr == "CLASS")
            {
                ok=parseSiteClass();
                ok = ok && expectSemicolonAndEOL();
            }
            else if (m_tokstr == "SYMMETRY")
            {
                ok=parseSiteSymmetry();
                ok = ok && expectSemicolonAndEOL();
            }
            else if (m_tokstr == "SIZE")
            {
                ok = parseSiteSize();
                ok = ok && expectSemicolonAndEOL();
            }
            else if (m_tokstr == "END")
            {
                endFound = true;
                m_curtok = tokenize(m_tokstr);

                // expect site name
                if (m_tokstr == name)
                {
                    onEndSite(name);
                    return true;
                }
                else
                {
                    ok = false;
                }
            }

            if (!ok)
            {
                std::stringstream ss;
                ss << "Error parsing SITE (token = " << m_tokstr << ")\n";
                error(ss.str());
                return false;
            }
        }

        if (atEnd())
        {
            error("Unexpected end of file\n");
            return false;
        }
    }
}

bool Parser::parsePin()
{
    std::string name;

    // pin name
    m_curtok = tokenize(name);
    if (m_curtok != TOK_IDENT)
    {
        error("Expected a pin name\n");
        return false;
    }

    // expect EOL
    m_curtok = tokenize(m_tokstr);
    if (m_curtok != TOK_EOL)
    {
        error("Expected EOL\n");
        return false;
    }

    onPin(name);

    // wait for 'END pinname'
    bool endFound = false;
    while(true)
    {
        m_curtok = tokenize(m_tokstr);

        if (m_curtok == TOK_IDENT)
        {
            if (m_tokstr == "DIRECTION")
            {
                parseDirection();
            }
            else if (m_tokstr == "USE")
            {
                parseUse();
            }
            else if (m_tokstr == "PORT")
            {
                onPort();
                parsePort();
            }
            else if (m_tokstr == "ANTENNAPARTIALMETALAREA")
            {
                skipUntilSemicolon();
            }
            else if (m_tokstr == "ANTENNAPARTIALMETALSIDEAREA")
            {
                skipUntilSemicolon();
            }
            else if (m_tokstr == "ANTENNAGATEAREA")
            {
                skipUntilSemicolon();
            }
        }

        if (endFound)
        {
            if ((m_curtok == TOK_IDENT) && (m_tokstr == name))
            {
                onEndPin(name);
                return true;
            }
        }
        else if ((m_curtok == TOK_IDENT) && (m_tokstr == "END"))
        {
            endFound = true;
        }
        else
        {
            endFound = false;
        }

        if (atEnd())
        {
            error("Unexpected end of file\n");
            return false;
        }
    }
}


bool Parser::parseObstruction()
{
    std::string name;

    // expect EOL
    m_curtok = tokenize(m_tokstr);
    if (m_curtok != TOK_EOL)
    {
        error("Expected EOL\n");
        return false;
    }

    onObstruction();

    // wait for 'END'
    bool endFound = false;
    while(true)
    {
        m_curtok = tokenize(m_tokstr);

        if (m_curtok == TOK_IDENT)
        {
            if (m_tokstr == "LAYER")
            {
                parseObstructionLayer();
            }
            else if (m_tokstr == "RECT")
            {
                parseRect();
            }
            else if (m_tokstr == "POLYGON")
            {
                parsePolygon();
            }
            else if (m_tokstr == "END")
            {
                OnEndObstruction();

                // expect EOL
                m_curtok = tokenize(m_tokstr);
                if (m_curtok != TOK_EOL)
                {
                    error("Expected EOL\n");
                    return false;
                }

                return true;
            }
        }

        if (atEnd())
        {
            error("Unexpected end of file\n");
            return false;
        }
    }
}

bool Parser::parseClass()
{
    // CLASS name <optional name> ';'

    std::string className;

    m_curtok = tokenize(m_tokstr);
    if (m_curtok != TOK_IDENT)
    {
        error("Expected at least one class in macro\n");
        return false;
    }
    className = m_tokstr;

    // check if there is a subclass or a semicolon
    m_curtok = tokenize(m_tokstr);
    if (m_curtok == TOK_SEMICOL)
    {
        // semicolon: end of line found -> exit
        onClass(className);
        return true;
    }

    if (m_curtok == TOK_IDENT)
    {
        // subclass
        // no semicolon checked yet -> continue.
        onClass(className, m_tokstr);
    }
    else
    {
        std::stringstream ss;
        ss << "Unexpected token " << m_tokstr << " (" << m_curtok << ")";
        error(ss.str());
        return false;
    }

    // check of end-of-line semicolon
    m_curtok = tokenize(m_tokstr);
    if (m_curtok != TOK_SEMICOL)
    {
        error("Expected a semicolon\n");
        return false;
    }

    return true;
};

bool Parser::parseOrigin()
{
    // ORIGIN <number> <number> ;


    std::string xnum;
    std::string ynum;

    m_curtok = tokenize(xnum);
    if (m_curtok != TOK_NUMBER)
    {
        error("parseOrigin: Expected a number\n");
        return false;
    }

    m_curtok = tokenize(ynum);
    if (m_curtok != TOK_NUMBER)
    {
        error("parseOrigin: Expected a number\n");
        return false;
    }

    m_curtok = tokenize(m_tokstr);
    if (m_curtok != TOK_SEMICOL)
    {
        error("parseOrigin: Expected a semicolon\n");
        return false;
    }

    bool ok = false;
    int64_t xnumd = flt2int(xnum, ok);
    if (!ok)
    {
        return false;
    }

    int64_t ynumd = flt2int(ynum, ok);
    if (!ok)
    {
        return false;
    }

    onOrigin(xnumd, ynumd);

    return true;
};

bool Parser::parseMacroSite()
{
    // SITE name ';'

    std::string siteName;

    m_curtok = tokenize(siteName);
    if (m_curtok != TOK_IDENT)
    {
        error("Expected an identifier\n");
        return false;
    }

    m_curtok = tokenize(m_tokstr);
    if (m_curtok != TOK_SEMICOL)
    {
        error("Expected a semicolon\n");
        return false;
    }

    onMacroSite(siteName);

    return true;
};

bool Parser::parseSize()
{
    // SIZE <number> BY <number> ';'

    std::string xnum;
    std::string ynum;

    m_curtok = tokenize(xnum);
    if (m_curtok != TOK_NUMBER)
    {
        error("parseSize: Expected a number\n");
        return false;
    }

    m_curtok = tokenize(m_tokstr);
    if ((m_curtok != TOK_IDENT) && (m_tokstr != "BY"))
    {
        error("parseSize: Expected 'BY'\n");
        return false;
    }

    m_curtok = tokenize(ynum);
    if (m_curtok != TOK_NUMBER)
    {
        error("parseSize: Expected a number\n");
        return false;
    }

    m_curtok = tokenize(m_tokstr);
    if (m_curtok != TOK_SEMICOL)
    {
        error("parseSize: Expected a semicolon\n");
        return false;
    }

    bool ok = false;
    int64_t xnumd = flt2int(xnum, ok);
    if (!ok)
    {
        return false;
    }

    int64_t ynumd = flt2int(ynum, ok);
    if (!ok)
    {
        return false;
    }

    onSize(xnumd, ynumd);

    return true;
};

bool Parser::parseSymmetry()
{
    // SYMMETRY (X|Y|R90)+ ';'

    std::string symmetry;

    // read options until we get to the semicolon.
    m_curtok = tokenize(m_tokstr);
    while(m_curtok!= TOK_SEMICOL)
    {
        symmetry += m_tokstr;
        symmetry += " ";
        m_curtok = tokenize(m_tokstr);
    }

    SymmetryFlags symflags;

    if (symmetry.find('X') != std::string::npos)
    {
        symflags += SymmetryFlags::SYM_X;
    }

    if (symmetry.find('Y') != std::string::npos)
    {
        symflags += SymmetryFlags::SYM_Y;
    }

    if (symmetry.find("R90") != std::string::npos)
    {
        symflags += SymmetryFlags::SYM_R90;
    }

    onSymmetry(symflags);
    return true;
};

bool Parser::parseForeign()
{
    // FOREIGN <cellname> [<number> <number>] ;

    std::string cellname;
    std::string xnum;
    std::string ynum;

    m_curtok = tokenize(cellname);
    if (m_curtok != TOK_IDENT)
    {
        error("parseForeign: Expected the cell name\n");
        return false;
    }

    // the statement can end here
    m_curtok = tokenize(xnum);
    if (m_curtok == TOK_SEMICOL)
    {
        onForeign(cellname, 0, 0);
        return true;
    }

    // or may continue with a number
    if (m_curtok != TOK_NUMBER)
    {
        error("parseForeign: Expected a number\n");
        return false;
    }

    m_curtok = tokenize(ynum);
    if (m_curtok != TOK_NUMBER)
    {
        error("parseForeign: Expected a number\n");
        return false;
    }

    m_curtok = tokenize(m_tokstr);
    if (m_curtok != TOK_SEMICOL)
    {
        error("parseForeign: Expected a semicolon\n");
        return false;
    }

    bool ok = false;
    int64_t xnumd = flt2int(xnum, ok);
    if (!ok)
    {
        return false;
    }

    int64_t ynumd = flt2int(ynum, ok);
    if (!ok)
    {
        return false;
    }

    onForeign(cellname, xnumd, ynumd);

    return true;
};


bool Parser::parseDirection()
{
    // DIRECTION OUTPUT/INPUT/INOUT etc.
    std::string direction;

    // read options until we get to the semicolon.
    m_curtok = tokenize(direction);
    if (m_curtok != TOK_IDENT)
    {
        error("Expected direction\n");
        return false;
    }

    m_curtok = tokenize(m_tokstr);
    if ((direction == "OUTPUT") && (m_tokstr == "TRISTATE"))
    {
        // OUTPUT can be followed by TRISTATE
        // FIXME: use enum.
        direction += " TRISTATE";
        m_curtok = tokenize(m_tokstr);
    }

    if (m_curtok != TOK_SEMICOL)
    {
        error("Expected a semicolon\n");
        return false;
    }

    onPinDirection(direction);

    return true;
};

bool Parser::parseUse()
{
    // USE OUTPUT/INPUT/INOUT etc.

    std::string use;

    m_curtok = tokenize(use);
    if (m_curtok != TOK_IDENT)
    {
        error("Expected use\n");
        return false;
    }

    m_curtok = tokenize(m_tokstr);
    if (m_curtok != TOK_SEMICOL)
    {
        error("Expected a semicolon\n");
        return false;
    }

    onPinUse(use);

    return true;
};

bool Parser::parsePort()
{
    std::string name;


    m_curtok = tokenize(m_tokstr);
    if (m_curtok != TOK_EOL)
    {
        error("Expected EOL\n");
        return false;
    }

    // expect LAYER
    m_curtok = tokenize(m_tokstr);
    while (m_curtok == TOK_IDENT)
    {
        if (m_tokstr == "LAYER")
        {
            parsePortLayer();
        }
        else if (m_tokstr == "END")
        {
            break;
        }
        else
        {
            // eat until ;
            do
            {
                m_curtok = tokenize(m_tokstr);
            } while(m_curtok != TOK_SEMICOL);

            // eat newline
            m_curtok = tokenize(m_tokstr);
            if (m_curtok != TOK_EOL)
            {
                error("Expected EOL");
                return false;
            }
        }

        if (atEnd())
        {
            error("Unexpected end of file\n");
            return false;
        }
    }

    onEndPort();
    return true;
}

bool Parser::parsePortLayer()
{
    // LAYER <name> ';'
    std::string name;

    m_curtok = tokenize(name);
    if (m_curtok != TOK_IDENT)
    {
        error("Expected layer name\n");
        return false;
    }

    onPortLayer(name);

    m_curtok = tokenize(m_tokstr);
    if (m_curtok != TOK_SEMICOL)
    {
        error("Expected a semicolon\n");
        return false;
    }

    m_curtok = tokenize(m_tokstr);
    if (m_curtok != TOK_EOL)
    {
        error("Expected an EOL\n");
        return false;
    }

    do
    {
        if (!parsePortLayerItem())
        {
            return false;
        }
    } while(m_tokstr != "END");

    return true;
}

bool Parser::parsePortLayerItem()
{
    m_curtok = tokenize(m_tokstr);
    if (m_curtok != TOK_IDENT)
    {
        error("Expected RECT or END\n");
        return false;
    }

    while(true)
    {
        if (m_tokstr == "END")
        {
            return true;
        }

        if (m_tokstr == "RECT")
        {
            parseRect();
        }
        else if (m_tokstr == "POLYGON")
        {
            parsePolygon();
        }

        m_curtok = tokenize(m_tokstr);
    }
}

bool Parser::parseRect()
{
    // expect: <number> <number> <number> <number> ;
    std::array<int64_t,4> coords = {0,0,0,0};

    for(auto& coord : coords)
    {
        m_curtok = tokenize(m_tokstr);

        if (m_curtok != TOK_NUMBER)
        {
            error("Expected number in RECT\n");
            return false;
        }

        bool ok = false;
        coord = flt2int(m_tokstr, ok);

        if (!ok)
        {
            error("Reading RECT coordinates\n");
            return false;
        }
    }

    // expect ;
    m_curtok = tokenize(m_tokstr);
    if (m_curtok != TOK_SEMICOL)
    {
        error("Expected a semicolon in RECT\n");
        return false;
    }

    // expect EOL
    m_curtok = tokenize(m_tokstr);
    if (m_curtok != TOK_EOL)
    {
        error("Expected an EOL in RECT\n");
        return false;
    }

    onRect(coords[0], coords[1], coords[2], coords[3]);

    return true;
}

bool Parser::parsePolygon()
{
    std::vector<Coord64> points;

    // keep on reader x,y coordinates until
    // we see a ';'

    m_curtok = tokenize(m_tokstr);
    while(m_curtok == TOK_NUMBER)
    {
        Coord64 p;
        bool ok = false;
        p.m_x = flt2int(m_tokstr, ok);
        if (!ok)
        {
            error("Cannot convert number to coordinate in POLYGON statement\n");
            return false;
        }

        m_curtok = tokenize(m_tokstr);
        if (m_curtok != TOK_NUMBER)
        {
            error("Expected a number in POLYGON statement\n");
            return false;
        }

        p.m_y = flt2int(m_tokstr, ok);
        if (!ok)
        {
            error("Cannot convert number to coordinate in POLYGON statement\n");
            return false;
        }

        points.push_back(p);

        m_curtok = tokenize(m_tokstr);
    };

    // check if we have at least 3 points
    if (points.size() < 3)
    {
        error("A LEF PLOYGON should have at least 3 points\n");
        return false;
    }

    // expect ;
    if (m_curtok != TOK_SEMICOL)
    {
        error("Expected a semicolon in POLYGON\n");
        return false;
    }

    // expect EOL
    m_curtok = tokenize(m_tokstr);
    if (m_curtok != TOK_EOL)
    {
        error("Expected an EOL in POLYGON\n");
        return false;
    }

    onPolygon(points);

    return true;
}

bool Parser::parseLayer()
{
    m_curtok = tokenize(m_tokstr);
    std::string layerName = m_tokstr;

    if (m_verbose)
    {
        std::cout << "  parseLayer " << layerName << "\n";
    }

    if (m_curtok != TOK_IDENT)
    {
        error("Expected a layer name\n");
        return false;
    }

    // expect EOL
    m_curtok = tokenize(m_tokstr);
    if (m_curtok != TOK_EOL)
    {
        error("Expected EOL in LAYER\n");
        return false;
    }

    onLayer(layerName);

    // parse all the layer items
    do
    {
        if (!parseLayerItem())
        {
            return false;
        }
    } while(m_tokstr != "END");

    // expect END <layername> EOL

    m_curtok = tokenize(m_tokstr);
    if (m_curtok != TOK_IDENT)
    {
        error("Expected END <layername>\n");
        return false;
    }

    if (m_tokstr != layerName)
    {
        error("Expected END <layername> : name does not match\n");
        return false;
    }

    m_curtok = tokenize(m_tokstr);

    if (m_curtok != TOK_EOL)
    {
        error("Expected EOL\n");
        return false;
    }

    onEndLayer(layerName);

    if (m_verbose)
    {
        std::cout << "  end parseLayer " << layerName << "\n";
    }

    return true;
}

bool Parser::parseObstructionLayer()
{
    // expect: LAYER <name> ;
    // where LAYER was already handled.

    m_curtok = tokenize(m_tokstr);
    std::string layerName = m_tokstr;

    if (m_curtok != TOK_IDENT)
    {
        error("Expected a layer name\n");
        return false;
    }

    // optionally: SPACING minSpacing | DESIGNRULEWIDTH value;
    m_curtok = tokenize(m_tokstr);
    if (m_tokstr == "SPACING")
    {
        //FIXME: ignore for now
        m_curtok = tokenize(m_tokstr);
        if (m_curtok != TOK_NUMBER)
        {
            error("Expected a number after SPACING\n");
            return false;
        }

        auto minSpacing = std::stof(m_tokstr);
        m_curtok = tokenize(m_tokstr);
    }
    else if (m_tokstr == "DESIGNRULEWIDTH")
    {
        //FIXME: ignore for now
        m_curtok = tokenize(m_tokstr);
        if (m_curtok != TOK_NUMBER)
        {
            error("Expected a number after DESIGNRULEWIDTH\n");
            return false;
        }

        auto value = std::stof(m_tokstr);
        m_curtok = tokenize(m_tokstr);
    }

    // expect ;
    if (m_curtok != TOK_SEMICOL)
    {
        error("Expected semicolon after OBS LAYER\n");
        return false;
    }

    // expect EOL
    m_curtok = tokenize(m_tokstr);
    if (m_curtok != TOK_EOL)
    {
        error("Expected EOL in OBS LAYER\n");
        return false;
    }

    onObstructionLayer(layerName);

    return true;
}

bool Parser::parseLayerItem()
{
    // eat empty lines.
    do
    {
        m_curtok = tokenize(m_tokstr);
    } while ((m_curtok == TOK_EOL) && (m_curtok != TOK_EOF));

    if (m_curtok != TOK_IDENT)
    {
        std::stringstream ss;
        ss << "Expected identifier in layer item but got '" << m_tokstr << "' token id " << m_curtok << "\n";
        error(ss.str());
        return false;
    }

    if (m_tokstr == "PITCH")
    {
        bool ok = parseLayerPitch();
        ok = ok && expectSemicolonAndEOL();
        return ok;
    }

    if (m_tokstr == "SPACING")
    {
        bool ok = parseLayerSpacing();
        ok = ok && expectSemicolonAndEOL();
        return ok;
    }

    if (m_tokstr == "OFFSET")
    {
        bool ok = parseLayerOffset();
        ok = ok && expectSemicolonAndEOL();
        return ok;
    }

    if (m_tokstr == "TYPE")
    {
        bool ok = parseLayerType();
        ok = ok && expectSemicolonAndEOL();
        return ok;
    }

    if (m_tokstr == "DIRECTION")
    {
        bool ok = parseLayerDirection();
        ok = ok && expectSemicolonAndEOL();
        return ok;
    }

    if (m_tokstr == "AREA")
    {
        // AREA is actually a spec for minimal area..
        bool ok = parseLayerMinArea();
        ok = ok && expectSemicolonAndEOL();
        return ok;
    }

    if (m_tokstr == "THICKNESS")
    {
        bool ok = parseLayerThickness();
        ok = ok && expectSemicolonAndEOL();
        return ok;
    }

    if (m_tokstr == "WIDTH")
    {
        bool ok = parseLayerWidth();
        ok = ok && expectSemicolonAndEOL();
        return ok;
    }

    if (m_tokstr == "MAXWIDTH")
    {
        bool ok = parseLayerMaxWidth();
        ok = ok && expectSemicolonAndEOL();
        return ok;
    }

    if (m_tokstr == "MINWIDTH")
    {
        bool ok = parseLayerMinWidth();
        ok = ok && expectSemicolonAndEOL();
        return ok;
    }

    if (m_tokstr == "RESISTANCE")
    {
        bool ok = parseLayerResistance();
        ok = ok && expectSemicolonAndEOL();
        return ok;
    }

    if (m_tokstr == "CAPACITANCE")
    {
        bool ok = parseLayerCapacitance();
        ok = ok && expectSemicolonAndEOL();
        return ok;
    }

    if (m_tokstr == "EDGECAPACITANCE")
    {
        bool ok = parseLayerEdgeCapacitance();
        ok = ok && expectSemicolonAndEOL();
        return ok;
    }

    if (m_tokstr == "SPACINGTABLE")
    {
        bool ok = parseLayerSpacingTable();
        ok = ok && expectSemicolonAndEOL();
        return ok;
    }

    if (m_tokstr == "END")
    {
        return true;
    }

    // eat everything on the line
    std::stringstream ss;
    ss << "  Skipping: " << m_tokstr;
    while((m_curtok != TOK_EOL) && (m_curtok != TOK_EOF))
    {
        m_curtok = tokenize(m_tokstr);
        ss << " " << m_tokstr;
    }
    ss << "\n";

    Logging::doLog(Logging::LogType::INFO, ss.str());
    if (m_curtok == TOK_EOF)
    {
        error("Unexpected end of file");
        return false;
    }

    return true;
}

bool Parser::parseLayerSpacing()
{
    // SPACING <number> ;
    // or
    // SPACING <number> ADJECENTCUTS <number> WITHIN <number> ;
    // SPACING <number> RANGE <number> <number> ;
    // SPACING <number> RANGE <number> <number> INFLUENCE <number> ;
    // SPACING <number> ENDOFLINE <number> WITHIN <number> ;

    std::string pitch;
    m_curtok = tokenize(m_tokstr);
    if (m_curtok != TOK_NUMBER)
    {
        error("Expected a number in layer spacing\n");
        return false;
    }

    bool ok = false;
    int64_t pitchd = flt2int(m_tokstr, ok);

    if (!ok)
    {
        return false;
    }

    onLayerSpacing(pitchd);

    // expect ; or RANGE
    m_curtok = tokenize(m_tokstr);
    if (m_tokstr == "RANGE")
    {
        parseLayerSpacingRange();
        m_curtok = tokenize(m_tokstr);

        // check for INFLUENCE
        if (m_tokstr == "INFLUENCE")
        {
            m_curtok = tokenize(m_tokstr);
            if (m_curtok != TOK_NUMBER)
            {
                error("Expected number after INFLUENCE");
            }

            int64_t influence = flt2int(m_tokstr, ok);
            onLayerSpacingRangeInfluence(influence);
            m_curtok = tokenize(m_tokstr);
        }
    }
    else if (m_tokstr == "ENDOFLINE")
    {
        //FIXME: ignore this for now
        m_curtok = tokenize(m_tokstr);
        if (m_curtok != TOK_NUMBER)
        {
            error("Expected number after ENDOFLINE");
            return false;
        }

        m_curtok = tokenize(m_tokstr);
        if (m_tokstr != "WITHIN")
        {
            error("Expected 'WITHIN'");
            return false;
        }

        m_curtok = tokenize(m_tokstr);
        if (m_curtok != TOK_NUMBER)
        {
            error("Expcted a number after WITHIN");
            return false;
        }

        m_curtok = tokenize(m_tokstr);
    }
    else if (m_tokstr == "ADJACENTCUTS")
    {
        //FIXME: ignore this for now
        m_curtok = tokenize(m_tokstr);
        if (m_curtok != TOK_NUMBER)
        {
            error("Expected 2, 3 or 4 after ADJACENTCUTS");
            return false;
        }

        auto numCuts = std::stod(m_tokstr);
        if ((numCuts < 2) || (numCuts > 4))
        {
            error("Expected 2, 3 or 4 after ADJACENTCUTS");
            return false;
        }

        m_curtok = tokenize(m_tokstr);
        if (m_tokstr != "WITHIN")
        {
            error("Expected 'WITHIN'");
            return false;
        }

        m_curtok = tokenize(m_tokstr);
        if (m_curtok != TOK_NUMBER)
        {
            error("Expcted a number after WITHIN");
            return false;
        }

        m_curtok = tokenize(m_tokstr);
    }
    return true;
}

bool Parser::parseLayerSpacingRange()
{
    // expect: number number
    m_curtok = tokenize(m_tokstr);

    if (m_curtok != TOK_NUMBER)
    {
        error("Expected number after RANGE\n");
        return false;
    }

    bool ok = false;
    int64_t value1 = flt2int(m_tokstr, ok);

    if (!ok)
    {
        return false;
    }

    m_curtok = tokenize(m_tokstr);

    if (m_curtok != TOK_NUMBER)
    {
        error("Expected number after RANGE\n");
        return false;
    }

    int64_t value2 = flt2int(m_tokstr, ok);

    onLayerSpacingRange(value1,value2);

    return true;
}

bool Parser::parseLayerPitch()
{
    //FIXME: use two strings, one may be empty
    std::string pitch;
    m_curtok = tokenize(m_tokstr);
    if (m_curtok != TOK_NUMBER)
    {
        error("Expected a number in layer pitch\n");
        return false;
    }

    pitch = m_tokstr;

    m_curtok = tokenize(m_tokstr);
    bool dualarg = false;
    std::string pitch2;

    // LEF 5.8 allows a second pitch
    if (m_curtok == TOK_NUMBER)
    {
        dualarg = true;
        pitch2 = m_tokstr;
        m_curtok = tokenize(m_tokstr);
    }

    bool ok = false;
    int64_t pitchd  = flt2int(pitch, ok);

    if (!ok)
    {
        return false;
    }

    if (dualarg)
    {
        int64_t pitchd2 = flt2int(pitch2, ok);
        onLayerPitch(pitchd, pitchd2);
    }
    else
    {
        onLayerPitch(pitchd, pitchd);
    }

    return true;
}


bool Parser::parseLayerOffset()
{
    std::string offsetx;
    std::string offsety;

    m_curtok = tokenize(m_tokstr);
    if (m_curtok != TOK_NUMBER)
    {
        error("Expected a number in layer offset\n");
        return false;
    }

    offsetx = m_tokstr;

    // check if we have a second dimension (y) offset
    m_curtok = tokenize(m_tokstr);
    if (m_curtok == TOK_NUMBER)
    {
        offsety = m_tokstr;
        m_curtok = tokenize(m_tokstr);
    }

    bool ok = false;
    int64_t offsetxd = flt2int(offsetx, ok);
    if (!ok)
    {
        return false;
    }

    if (offsety.empty())
    {
        // only one dimension is given in the LEF file
        // and applies to both x and y directions
        onLayerOffset(offsetxd, offsetxd);
    }
    else
    {
        int64_t offsetyd = flt2int(offsety, ok);
        onLayerOffset(offsetxd, offsetyd);
    }

    return true;
}

bool Parser::parseLayerType()
{
    std::string layerType;
    m_curtok = tokenize(m_tokstr);
    if (m_curtok != TOK_IDENT)
    {
        error("Expected a string in layer type\n");
        return false;
    }

    layerType = m_tokstr;

    onLayerType(layerType);

    // read ';'
    m_curtok = tokenize(m_tokstr);

    return true;
}

bool Parser::parseLayerWidth()
{
    std::string width;
    m_curtok = tokenize(m_tokstr);
    if (m_curtok != TOK_NUMBER)
    {
        error("Expected a number in layer width\n");
        return false;
    }

    width = m_tokstr;

    bool ok = false;
    int64_t widthd = flt2int(width, ok);
    if (!ok)
    {
        return false;
    }

    onLayerWidth(widthd);

    // read ';'
    m_curtok = tokenize(m_tokstr);

    return true;
}

bool Parser::parseLayerMaxWidth()
{
    std::string maxwidth;
    m_curtok = tokenize(m_tokstr);
    if (m_curtok != TOK_NUMBER)
    {
        error("Expected a number in layer max width\n");
        return false;
    }

    maxwidth = m_tokstr;

    bool ok = false;
    int64_t maxwidthd = flt2int(maxwidth, ok);
    if (!ok)
    {
        return false;
    }

    onLayerMaxWidth(maxwidthd);

    // read ';'
    m_curtok = tokenize(m_tokstr);

    return true;
}


bool Parser::parseLayerMinWidth()
{
    std::string minwidth;
    m_curtok = tokenize(m_tokstr);
    if (m_curtok != TOK_NUMBER)
    {
        error("Expected a number in layer min width\n");
        return false;
    }

    minwidth = m_tokstr;

    bool ok = false;
    int64_t minwidthd = flt2int(minwidth, ok);
    if (!ok)
    {
        return false;
    }

    onLayerMinWidth(minwidthd);

    // read ';'
    m_curtok = tokenize(m_tokstr);

    return true;
}


bool Parser::parseLayerDirection()
{
    std::string direction;
    m_curtok = tokenize(m_tokstr);
    if (m_curtok != TOK_IDENT)
    {
        error("Expected a string in layer direction\n");
        return false;
    }

    direction = m_tokstr;

    onLayerDirection(direction);

    // read ';'
    m_curtok = tokenize(m_tokstr);

    return true;
}

bool Parser::parseSiteItem()
{
    // eat empty lines.
    do
    {
        m_curtok = tokenize(m_tokstr);
    } while ((m_curtok == TOK_EOL) && (m_curtok != TOK_EOF));

    if (m_curtok != TOK_IDENT)
    {
        std::stringstream ss;
        ss << "Expected identifier in SITE item but got '" << m_tokstr << "' token id " << m_curtok << "\n";
        error(ss.str());
        return false;
    }

    if (m_tokstr == "CLASS")
    {
        bool ok = parseSiteClass();
        ok = ok && expectSemicolonAndEOL();
        return ok;
    }

    if (m_tokstr == "SYMMETRY")
    {
        bool ok = parseSiteSymmetry();
        ok = ok && expectSemicolonAndEOL();
        return ok;
    }

    if (m_tokstr == "SIZE")
    {
        bool ok = parseSiteSize();
        ok = ok && expectSemicolonAndEOL();
        return ok;
    }

    if (m_tokstr == "END")
    {
        // eat the site name
        m_curtok = tokenize(m_tokstr);
        return true;
    }


    // eat everything on the line
    std::stringstream ss;
    ss << "  Skipping: " << m_tokstr;
    while((m_curtok != TOK_EOL) && (m_curtok != TOK_EOF))
    {
        m_curtok = tokenize(m_tokstr);
        ss << " " << m_tokstr;
    }
    ss << "\n";

    Logging::doLog(Logging::LogType::INFO, ss.str());
    if (m_curtok == TOK_EOF)
    {
        error("Unexpected end of file");
        return false;
    }

    return true;
}

bool Parser::parseSiteClass()
{
    // CLASS name ';'

    m_curtok = tokenize(m_tokstr);
    if (m_curtok != TOK_IDENT)
    {
        error("Expected SITE CLASS name\n");
        return false;
    }

    std::string className = toUpper(m_tokstr);

    onSiteClass(className);

    // read ;
    m_curtok = tokenize(m_tokstr);

    if (m_curtok != TOK_SEMICOL)
    {
        error("Expected ; after Site class\n");
        return false;
    }

    return true;
}

bool Parser::parseSiteSymmetry()
{
    // SYMMETRY (X|Y|R90)+ ';'

    std::string symmetry;

    // read options until we get to the semicolon.
    m_curtok = tokenize(m_tokstr);
    while(m_curtok!= TOK_SEMICOL)
    {
        symmetry += m_tokstr;
        symmetry += " ";
        m_curtok = tokenize(m_tokstr);
    }

    symmetry = toUpper(symmetry);

    SymmetryFlags flags;
    flags.m_flags = SymmetryFlags::SYM_NONE;

    if (symmetry.find('X') != std::string::npos)
    {
        flags.m_flags |= SymmetryFlags::SYM_X;
    }

    if (symmetry.find('Y') != std::string::npos)
    {
        flags.m_flags |= SymmetryFlags::SYM_Y;
    }

    if (symmetry.find("R90") != std::string::npos)
    {
        flags.m_flags |= SymmetryFlags::SYM_R90;
    }

    onSiteSymmetry(flags);
    return true;
}


bool Parser::parseSiteSize()
{
    // SIZE <number> BY <number> ';'

    std::string xnum;
    std::string ynum;

    m_curtok = tokenize(xnum);
    if (m_curtok != TOK_NUMBER)
    {
        error("Expected a number\n");
        return false;
    }

    m_curtok = tokenize(m_tokstr);
    if ((m_curtok != TOK_IDENT) && (m_tokstr != "BY"))
    {
        error("Expected 'BY'\n");
        return false;
    }

    m_curtok = tokenize(ynum);
    if (m_curtok != TOK_NUMBER)
    {
        error("Expected a number\n");
        return false;
    }

    bool ok = false;
    int64_t xnumd = flt2int(xnum, ok);
    if (!ok)
    {
        return false;
    }

    int64_t ynumd = flt2int(ynum, ok);
    if (!ok)
    {
        return false;
    }

    onSiteSize(xnumd, ynumd);

    // read ;
    m_curtok = tokenize(m_tokstr);

    return true;
}

bool Parser::parseVia()
{
    // VIA <vianame> ...
    // keep on reading tokens until we
    // find END <vianame>

    if (m_verbose)
    {
        std::cout << "  parseVia ";
    }

    m_curtok = tokenize(m_tokstr);
    std::string viaName;

    if (m_curtok != TOK_IDENT)
    {
        error("Expected identifier in via name\n");
        return false;
    }

    viaName = m_tokstr;
    if (m_verbose)
    {
        std::cout << viaName << "\n";
    }

    // read until we get END <vianame>
    do
    {
        m_curtok = tokenize(m_tokstr);
        while(m_tokstr != "END")
        {
            m_curtok = tokenize(m_tokstr);
        }

        // read via name
        m_curtok = tokenize(m_tokstr);
        if (m_curtok != TOK_IDENT)
        {
            error("Expected via name after END\n");
            return false;
        }
    } while(m_tokstr != viaName);

    m_curtok = tokenize(m_tokstr);
    if (m_curtok != TOK_EOL)
    {
        error("Expected EOL after END <vianame>\n");
        return false;
    }

    onVia(viaName);

    if (m_verbose)
    {
        std::cout << "  end parseVia " << viaName << "\n";
    }

    return true;
}

bool Parser::parseViaRule()
{
    // VIARULE <vianame> ...
    // keep on reading tokens until we
    // find END <vianame>

    m_curtok = tokenize(m_tokstr);
    std::string viaRuleName;

    if (m_curtok != TOK_IDENT)
    {
        error("Expected identifier in viarule name\n");
        return false;
    }

    viaRuleName = m_tokstr;

    // read until we get END <vianame>
    do
    {
        m_curtok = tokenize(m_tokstr);
        while(m_tokstr != "END")
        {
            m_curtok = tokenize(m_tokstr);
        }

        // read via name
        m_curtok = tokenize(m_tokstr);
        if (m_curtok != TOK_IDENT)
        {
            error("Expected viarule name after END\n");
            return false;
        }
    } while(m_tokstr != viaRuleName);

    m_curtok = tokenize(m_tokstr);
    if (m_curtok != TOK_EOL)
    {
        error("Expected EOL after END <viarule name>\n");
        return false;
    }

    onViaRule(viaRuleName);

    return true;
}

bool Parser::parseUnits()
{
    // UNITS
    //   DATABASE MICRONS ;
    //   ...
    // END UNITS

    // expect EOL after UNITS

    m_curtok = tokenize(m_tokstr);
    if (m_curtok != TOK_EOL)
    {
        error("Expected EOL after UNITS\n");
        return false;
    }

    while(true)
    {
        m_curtok = tokenize(m_tokstr);

        if (m_curtok != TOK_IDENT)
        {
            error("Expected string in units block\n");
            return false;
        }

        if (m_tokstr == "DATABASE")
        {
            m_curtok = tokenize(m_tokstr);
            if ((m_curtok == TOK_IDENT) && (m_tokstr == "MICRONS"))
            {
                m_curtok = tokenize(m_tokstr);
                if (m_curtok == TOK_NUMBER)
                {
                    double micronsd = 0.0;
                    try
                    {
                        micronsd = std::stod(m_tokstr);
                    }
                    catch(const std::invalid_argument& ia)
                    {
                        error(ia.what());
                        return false;
                    }

                    m_dBMicrons = static_cast<float>(micronsd);

                    // expect ; and EOL
                    m_curtok = tokenize(m_tokstr);
                    if (m_curtok != TOK_SEMICOL)
                    {
                        error("Expected ; after DATABASE MICRONS <number>\n");
                        return false;
                    }

                    m_curtok = tokenize(m_tokstr);
                    if (m_curtok != TOK_EOL)
                    {
                        error("Expected EOL after DATABASE MICRONS <number> ;\n");
                        return false;
                    }

                    onDatabaseUnitsMicrons(micronsd);
                }
                else
                {
                    error("Expected a number after DATABASE MICRONS\n");
                    return false;
                }
            }
            else
            {
                error("Expects MICRONS keywords after DATABASE\n");
                return false;
            }
        }
        else if (m_tokstr == "END")
        {
            // check for units
            m_curtok = tokenize(m_tokstr);
            if ((m_curtok == TOK_IDENT) && (m_tokstr == "UNITS"))
            {
                return true;
            }

            // read until EOL
            while ((m_curtok != TOK_EOL) && (m_curtok != TOK_EOF))
            {
                m_curtok = tokenize(m_tokstr);
            }
        }
        else
        {
            // got something other than DATABASE or END
            // read until EOL
            while ((m_curtok != TOK_EOL) && (m_curtok != TOK_EOF))
            {
                m_curtok = tokenize(m_tokstr);
            }
        }
    }
}

bool Parser::parsePropertyDefintions()
{
    // basically, eat everything until
    // we encounter END PROPERTYDEFINTIONS EOL

    while(true)
    {
        m_curtok = tokenize(m_tokstr);
        if ((m_curtok == TOK_IDENT) && (m_tokstr == "END"))
        {
            m_curtok = tokenize(m_tokstr);
            if ((m_curtok == TOK_IDENT) && (m_tokstr == "PROPERTYDEFINITIONS"))
            {
                m_curtok = tokenize(m_tokstr);

                if (m_curtok == TOK_EOL)
                {
                    return true;
                }

                if (m_curtok == TOK_EOF)
                {
                    error("Unexpected end of liberty file\n");
                    return false;
                }
            }
            else if (m_curtok == TOK_EOF)
            {
                error("Unexpected end of liberty file\n");
                return false;
            }
        }
        else if (m_curtok == TOK_EOF)
        {
            error("Unexpected end of liberty file\n");
            return false;
        }
    }
}

bool Parser::parseManufacturingGrid()
{
    m_curtok = tokenize(m_tokstr);
    if (m_curtok != TOK_NUMBER)
    {
        error("Expected a number after MANUFACTURINGGRID\n");
        return false;
    }

    // manufacturing grid is _always_ specified in microns
    double value = 0.0;
    try
    {
        value = std::stod(m_tokstr);
    }
    catch(const std::invalid_argument& ia)
    {
        error(ia.what());
        return false;
    }

    // expect ; and EOL
    m_curtok = tokenize(m_tokstr);
    if (m_curtok != TOK_SEMICOL)
    {
        error("Expected ; after MANUFACTURINGGRID <number>\n");
        return false;
    }

    m_curtok = tokenize(m_tokstr);
    if (m_curtok != TOK_EOL)
    {
        error("Expected EOL after MANUFACTURINGGRID <number> ;\n");
        return false;
    }

    onManufacturingGrid(value);

    return true;
}

bool Parser::parseLayerResistance()
{
    // if this is a CUT type layer,
    // the resistance does not have a 'RPERSQ' token
    // but is directly followed by a number
    m_curtok = tokenize(m_tokstr);

    if (m_curtok == TOK_NUMBER)
    {
        // assume this is a CUT type layer and accept the
        // resistance as-is.
        double resistance = std::stod(m_tokstr);
        onLayerResistance(resistance);

        m_curtok = tokenize(m_tokstr);
        return true;
    }

    if ((m_curtok != TOK_IDENT) || (m_tokstr != "RPERSQ"))
    {
        error("Expected RPERSQ after RESISTANCE\n");
        return false;
    }

    m_curtok = tokenize(m_tokstr);
    if (m_curtok != TOK_NUMBER)
    {
        error("Expected NUMBER after RPERSQ\n");
        return false;
    }

    double resistance = std::stod(m_tokstr);

    // according to the LEF 5.8 spec,
    // these figures are always in Ohms.
    onLayerResistancePerSq(resistance);

    m_curtok = tokenize(m_tokstr);

    return true;
}

bool Parser::parseLayerEdgeCapacitance()
{
    m_curtok = tokenize(m_tokstr);
    if (m_curtok != TOK_NUMBER)
    {
        error("Expected NUMBER after EDGECAPACITANCE\n");
        return false;
    }

    double picoFarads = std::stod(m_tokstr);

    // according to the LEF 5.8 spec,
    // these figures are always in pico Farads.
    const double picoFaradUnit = 1.0e-12;
    onLayerEdgeCapacitance(picoFarads*picoFaradUnit);

    m_curtok = tokenize(m_tokstr);

    return true;
}

bool Parser::parseLayerCapacitance()
{
    m_curtok = tokenize(m_tokstr);
    if ((m_curtok != TOK_IDENT) || (m_tokstr != "CPERSQDIST"))
    {
        error("Expected CPERSQDIST after CAPACITANCE\n");
        return false;
    }

    m_curtok = tokenize(m_tokstr);
    if (m_curtok != TOK_NUMBER)
    {
        error("Expected NUMBER after CPERSQDIST\n");
        return false;
    }

    double picoFarads = std::stod(m_tokstr);

    // according to the LEF 5.8 spec,
    // these figures are always in picofarads.
    const double picoFaradUnit = 1.0e-12;
    onLayerCapacitancePerSq(picoFarads*picoFaradUnit);

    m_curtok = tokenize(m_tokstr);

    return true;
}

bool Parser::parseLayerSpacingTable()
{
    // SPACINGTABLE spans multiple line
    // and is terminated with a ';'
    //
    // at this time, we don't support a SPACINGTABLE

    while((m_curtok != TOK_SEMICOL) && (m_curtok != TOK_EOF))
    {
        m_curtok = tokenize(m_tokstr);
    };

    Logging::doLog(Logging::LogType::VERBOSE, "Skipping SPACINGTABLE in layer\n");

    // when we end up here,
    // we either have a TOK_SEMICOL or TOK_EOF

    return true;
}

bool Parser::parseLayerMinArea()
{
    m_curtok = tokenize(m_tokstr);
    if (m_curtok != TOK_NUMBER)
    {
        error("Expected NUMBER after AREA\n");
        return false;
    }

    // minimum metal area in microns squared
    double minArea = std::stod(m_tokstr);

    onLayerMinArea(minArea);

    // read ;
    m_curtok = tokenize(m_tokstr);

    return true;
}

bool Parser::parseLayerThickness()
{
    m_curtok = tokenize(m_tokstr);
    if (m_curtok != TOK_NUMBER)
    {
        error("Expected NUMBER after THICKNESS\n");
        return false;
    }

    // layer thickness in microns ??
    double thickness = std::stod(m_tokstr);

    onLayerThickness(thickness);

    // read ;
    m_curtok = tokenize(m_tokstr);

    return true;
}

bool Parser::expectSemicolonAndEOL()
{
    if (m_curtok != TOK_SEMICOL)
    {
        std::stringstream ss;
        ss << "Expected a semicolon but got " << m_tokstr << " (tok=" << m_curtok << ")";
        error(ss.str());
        //BREAK_HERE;
        return false;
    }

    m_curtok = tokenize(m_tokstr);
    if (m_curtok != TOK_EOL)
    {
        error("Exected an EOL");
        return false;
    }

    return true;
}
