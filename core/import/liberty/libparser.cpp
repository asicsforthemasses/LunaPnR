/*
    Copyright (c) 2020,2021 Niels Moseley <n.a.moseley@moseleyinstruments.com>

    Permission to use, copy, modify, and/or distribute this software for any
    purpose with or without fee is hereby granted, provided that the above
    copyright notice and this permission notice appear in all copies.

    THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
    WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
    MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
    ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
    WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
    ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
    OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
    
*/

#include <sstream>
#include <array>
#include <algorithm>
#include "common/logging.h"
#include "libparser.h"

using namespace ChipDB::Liberty;

bool Parser::isWhitespace(char c) const
{
    return ((c==' ') || (c == '\t'));
}

bool Parser::isAlpha(char c) const
{
    if (((c >= 'A') && (c <= 'Z')) || ((c >= 'a') && (c <= 'z')))
        return true;

    if ((c == '_') || (c == '!'))
        return true;

    return false;
}

bool Parser::isDigit(char c) const
{
    return ((c >= '0') && (c <= '9'));
}

bool Parser::isAlphaNumeric(char c) const
{
    return (isAlpha(c) || isDigit(c));
}

bool Parser::isExtendedAlphaNumeric(char c) const
{
    return (isAlpha(c) || isDigit(c) || (c==']') || (c=='[') || (c=='.') || (c==':'));
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


Parser::token_t Parser::tokenize(std::string &tokstr)
{
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
    if (match(10))
    {
        m_lineNum++;
        m_col = 1;        
        match(13);
        return TOK_EOL;
    }
    else if (match(13))
    {
        m_lineNum++;
        m_col = 1;        
        match(10);
        return TOK_EOL;
    }

    // check for line comment
    if (match('/'))    
    {
        if (match('*'))
        {
            bool commentEnd = false;
            while(!commentEnd)
            {
                if (atEnd())
                    return TOK_EOF;

                // skip new lines
                if (match(10))
                {
                    m_lineNum++;
                    m_col = 1;        
                    match(13);
                }
                else if (match(13))
                {
                    m_lineNum++;
                    m_col = 1;        
                    match(10);                
                }
                else if (match('*'))
                {
                    if (match('/'))
                    {
                        commentEnd = true;
                    }
                }
                else
                {
                    advance();
                }
            }
            return TOK_COMMENT;
        }
        return TOK_SLASH;
    }

    if (match('\\'))
    {
        return TOK_BSLASH; 
    }

    if (match(';'))
    {
        return TOK_SEMICOL; 
    }

    if (match(':'))
    {
        return TOK_COLON; 
    }

    if (match(','))
    {
        return TOK_COMMA; 
    }

    if (match('*'))
    {
        return TOK_STAR; 
    }

    if (match('+'))
    {
        return TOK_PLUS; 
    }

    if (match('{'))
    {
        return TOK_LCURLY;
    }

    if (match('}'))
    {
        return TOK_RCURLY;
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

            // it could be a unit, in which case it is directly followed by
            // some letters
            //c = peek();
            //while(isAlphaNumeric(c))
            //{                
            //    tokstr += c;
            //    advance();
            //    c = peek();
            //}

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

    // Quoted string. '\' is a line continuation
    // escape char. *sigh*
    if (match('"'))
    {
        bool lastCharWasBackslash = false;
        char c = peek();
        while(c != '"')
        {
            // if the previous char was a back slash
            // it might be a line continuation escape
            // char, so we check for any combination
            // of LF and CR and ignore them.
            if (lastCharWasBackslash)
            {
                if (c == 10)
                {
                    // discard the line ending.
                    advance();
                    c = peek();
                    if (c == 13)
                    {
                        advance();
                        c = peek();
                    }
                }
                else if (c == 13)
                {
                    // discard the line ending.
                    advance();
                    c = peek();
                    if (c == 10)
                    {
                        advance();
                        c = peek();
                    }                    
                }                
            }
            else
            {
                // if we see CR or LF,
                // this is the end of the string
                // which we should never see
                // inside a string!
                if ((c==10) || (c==13))
                {
                    error("Missing closing \" in string.");
                    return TOK_ERR;
                }
            }
            
            tokstr += c;
            lastCharWasBackslash = (c == '\\');
            advance();            
            c = peek();
        }

        // skip closing quotes
        if (!match('"'))
        {
            error("Missing closing \" in string.");
            return TOK_ERR;
        }

        // error on newline
        //c = peek();
        //if ((c == 10) || (c == 13))
        //{
        //    error("Missing closing \" in string.");
        //    return TOK_ERR;
        //}

        return TOK_STRING;
    }

    // Number
    if (isDigit(peek()))
    {
        tokstr = peek();
        
        advance();
        char c = peek();

        //FIXME: overly relaxed float parsing.
        while(isDigit(c) || (c == '.') || (c == 'e') || (c == '+') || (c == '-'))
        {            
            tokstr += c;
            advance();
            c = peek();
        }
        bool isString = false;
        while(isExtendedAlphaNumeric(c))
        {
            tokstr += c;
            advance();
            c = peek();
            isString = true;
        }
        return isString ? TOK_STRING : TOK_NUMBER;
    }

    return TOK_ERR;
}

bool Parser::parse(const std::string &libertyString)
{
    m_src = &libertyString;
    m_idx = 0;
    m_col = 1;
    m_lineNum = 1;
    
    m_tokstr.clear();
    
    m_curtok = TOK_EOF;
    do
    {   
        advanceToken();
        if (peekToken() == TOK_ERR)
        {
            doLog(LOG_ERROR, "Liberty parse error, current character = '%c' (0x%02X)\n", peek(), static_cast<uint32_t>(peek()));
            error("");
            return false;
        }
 
        parseStatement();

        // try to see if we have a DEFINE, ATTRIBUTE or GROUP
        // statement

    } while(m_curtok != TOK_EOF);

    onEndParse();
 
    return true;
}

void Parser::error(const std::string &errstr)
{
    std::stringstream ss;
    ss << "Line " << m_lineNum << " col " << m_col << " : " << errstr << "\n"; 
    ss << "      tok = " << m_tokstr << " id=" << m_curtok << "\n";
    doLog(LOG_ERROR, ss.str());
}

bool Parser::acceptToken(const token_t tok)
{
    if (m_curtok == tok)
    {
        advanceToken();
        return true;
    }
    return false;
}

void Parser::advanceToken()
{
    do
    {
        m_curtok = tokenize(m_tokstr);
    } while (m_curtok == TOK_COMMENT);
}

bool Parser::parseStatement()
{
    while(acceptToken(TOK_EOL)) {};

    std::string name = m_tokstr;

    if (!acceptToken(TOK_IDENT))
    {
        return false;
    }

    if (name == "define")
    {
        // define statement
        return parseDefine();
    }
    else if (acceptToken(TOK_LPAREN))
    {
        return parseGroupOrComplexAttribute(name);
    }
    else if (acceptToken(TOK_COLON))
    {
        // simple attribute <name> : <value>
        return parseSimpleAttribute(name);
    }

    // error
    return false;
}

bool Parser::parseList(std::vector<std::string> &list)
{
    // ignore line-continuation breaks '\' in lists.
    while(acceptToken(TOK_BSLASH))
    {
        if (!acceptToken(TOK_EOL))
        {
            error("Expected EOL after line continuation marker.");
        }
    } 

    // a parameter can be an identifier, a string, or a number

    std::string value = m_tokstr;
    if (acceptToken(TOK_IDENT) || acceptToken(TOK_STRING) || acceptToken(TOK_NUMBER))
    {
        list.push_back(value);
    }
    else
    {
        // lists can be empty
        return true;
    }

    while(acceptToken(TOK_BSLASH))
    {
        if (!acceptToken(TOK_EOL))
        {
            error("Expected EOL after line continuation marker.");
        }
    } 

    while(acceptToken(TOK_COMMA))
    {
        while(acceptToken(TOK_BSLASH))
        {
            if (!acceptToken(TOK_EOL))
            {
                error("Expected EOL after line continuation marker.");
            }
        } 

        value = m_tokstr;
        if (acceptToken(TOK_IDENT) || acceptToken(TOK_STRING) || acceptToken(TOK_NUMBER))
        {
            list.push_back(value);
        }        
        else
        {
            error("Exected an identifier, string or number in list\n");
            return false;
        }

        while(acceptToken(TOK_BSLASH))
        {
            if (!acceptToken(TOK_EOL))
            {
                error("Expected EOL after line continuation marker.");
            }
        }
    }
    return true;
}

bool Parser::parseGroupOrComplexAttribute(const std::string &group)
{
    //
    //  Group: group_name ([name]) { .. }
    //
    //  Complex attribute: attribute_name ( param1 [, param2 , param3 .. ] ) ;
    //
    //  name and LPAREN have already been parsed.
    //

    std::vector<std::string> params;

    bool isGroup = true;

    parseList(params);

    if (!acceptToken(TOK_RPAREN))
    {
        error("Expected ')' in group or complex attribute");
        return false;
    }

    // if we have curly brackets, it's a group
    if (acceptToken(TOK_LCURLY))
    {
        if (params.size() == 0)
        {
            onGroup(group);
        }
        else
        {
            onGroup(group, params[0]);
        }

        while(!atEnd())
        {
            // eat EOL here because the RCURLY is often on
            // a new line
            while(acceptToken(TOK_EOL)) {}
            
            if (acceptToken(TOK_RCURLY))
            {                    
                onEndGroup();
                return true;
            }
            else
            {
                if (!parseStatement())
                    return false;
            }
        }
        error("Unexpectedly reached the end of file.");
        return false;
    }
    else
    {
        // sometimes an EOL is used instead of ;
        // we will reluctantly accept it..
        //
        // note: we don't get the next token,
        // because the EOL is used later..

        if (m_curtok == TOK_EOL)
        {
            // complex attribute
            onComplexAttribute(group, params);
            return true;            
        }

        // check for closing ';'
        if (!acceptToken(TOK_SEMICOL))
        {
            error("Expected a ';' at end of complex attribute.");
        }

        // complex attribute
        onComplexAttribute(group, params);
        return true;
    }
}

#if 0
static bool isSimpleValueToken(Parser::token_t tok)
{
    if (tok == Parser::TOK_IDENT) return true;
    if (tok == Parser::TOK_STRING) return true;
    if (tok == Parser::TOK_NUMBER) return true;
    if (tok == Parser::TOK_SLASH) return true;
    if (tok == Parser::TOK_STAR) return true;
    if (tok == Parser::TOK_MINUS) return true;
    if (tok == Parser::TOK_PLUS) return true;
    return false;
};
#endif

bool Parser::parseSimpleAttribute(const std::string &name)
{
    // name and ':' already accepted
    // still to do: value ';'
    //
    //
    // I've seen expressions as 'values', such as "0.3 * VDD"
    // so we'll also have to accept that.. :-/
    // 

    constexpr std::array<token_t, 7> matchList =
        {TOK_IDENT, TOK_STRING, TOK_NUMBER, TOK_SLASH, TOK_STAR, TOK_MINUS, TOK_PLUS};

    std::string value = "";
    size_t itemCount = 0;
    while(std::find(matchList.begin(), matchList.end(), m_curtok) != matchList.end())
    //while(isSimpleValueToken(m_curtok))
    {   
        value += m_tokstr;
        itemCount++;
        advanceToken();
    };

    // sometimes simple attributes do not have an ';'
    // and we just accept an EOL as terminator
    // but we should not advance the token
    // as the EOL is expected after this
    // function returns.
    if (m_curtok == TOK_EOL)
    {
        onSimpleAttribute(name, value);
        return true;        
    }

    // check for closing ';'
    if (!acceptToken(TOK_SEMICOL))
    {
        error("Expected a ';' at end of simple attribute.");
    }

    onSimpleAttribute(name, value);
    return true;    
}

bool Parser::parseDefine()
{
    return false;
}

#if 0
bool LEFParser::parseMacro()
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
                parseSite();
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

bool LEFParser::parsePin()
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

    // wait for 'END macroname'
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


bool LEFParser::parseObstruction()
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

bool LEFParser::parseClass()
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
    else if (m_curtok == TOK_IDENT)
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

bool LEFParser::parseOrigin()
{
    // ORIGIN <number> <number> ; 

    
    std::string xnum;
    std::string ynum;

    m_curtok = tokenize(xnum);
    if (m_curtok != TOK_NUMBER)
    {
        error("Expected a number\n");
        return false;
    }

    m_curtok = tokenize(ynum);
    if (m_curtok != TOK_NUMBER)
    {
        error("Expected a number\n");
        return false;
    }

    m_curtok = tokenize(m_tokstr);
    if (m_curtok != TOK_SEMICOL)
    {
        error("Expected a semicolon\n");
        return false;
    }

    bool ok;
    int64_t xnumd = flt2int(xnum, ok);
    int64_t ynumd = flt2int(ynum, ok);

    onOrigin(xnumd, ynumd);

    return true;
};

bool LEFParser::parseSite()
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

    onSite(siteName);

    //std::cout << "  SITE " << siteName << "\n";

    return true;
};

bool LEFParser::parseSize()
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

    m_curtok = tokenize(m_tokstr);
    if (m_curtok != TOK_SEMICOL)
    {
        error("Expected a semicolon\n");
        return false;
    }

    bool ok;
    int64_t xnumd = flt2int(xnum, ok);
    int64_t ynumd = flt2int(ynum, ok);

    onSize(xnumd, ynumd);

    return true;
};

bool LEFParser::parseSymmetry()
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

    bool flipv = false;
    bool fliph = false;

    if (symmetry.find('X') != symmetry.npos)
        fliph = true;

    if (symmetry.find('Y') != symmetry.npos)
        flipv = true;

    onSymmetry(fliph, flipv);
    return true;
};

bool LEFParser::parseForeign()
{
    // FOREIGN <cellname> <number> <number> ; 

    std::string cellname;
    std::string xnum;
    std::string ynum;

    m_curtok = tokenize(cellname);
    if (m_curtok != TOK_IDENT)
    {
        error("Expected the cell name\n");
        return false;
    }

    m_curtok = tokenize(xnum);
    if (m_curtok != TOK_NUMBER)
    {
        error("Expected a number\n");
        return false;
    }

    m_curtok = tokenize(ynum);
    if (m_curtok != TOK_NUMBER)
    {
        error("Expected a number\n");
        return false;
    }

    m_curtok = tokenize(m_tokstr);
    if (m_curtok != TOK_SEMICOL)
    {
        error("Expected a semicolon\n");
        return false;
    }

    bool ok;
    int64_t xnumd = flt2int(xnum, ok);
    int64_t ynumd = flt2int(ynum, ok);

    onForeign(cellname, xnumd, ynumd);

    return true;
};


bool LEFParser::parseDirection()
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

bool LEFParser::parseUse()
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

bool LEFParser::parsePort()
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

bool LEFParser::parsePortLayer()
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

bool LEFParser::parsePortLayerItem()
{
    m_curtok = tokenize(m_tokstr);
    if (m_curtok != TOK_IDENT)
    {
        error("Expected RECT or END\n");
        return false;
    }

    while(1)
    {
        if (m_tokstr == "END")
        {
            return true;
        }
        else if (m_tokstr == "RECT")
        {
            parseRect();
        }
        m_curtok = tokenize(m_tokstr);
    }
}

bool LEFParser::parseRect()
{   
    // expect: <number> <number> <number> <number> ;
    int64_t coords[4];

    for(uint32_t i=0; i<4; i++)
    {
        m_curtok = tokenize(m_tokstr);

        if (m_curtok != TOK_NUMBER)
        {
            error("Expected number in RECT\n");
            return false;
        }

        bool ok;
        coords[i] = flt2int(m_tokstr, ok);
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

bool LEFParser::parseLayer()
{
    m_curtok = tokenize(m_tokstr);
    std::string layerName = m_tokstr;

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

    return true;
}

bool LEFParser::parseObstructionLayer()
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

    // expect ;
    m_curtok = tokenize(m_tokstr);
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

bool LEFParser::parseLayerItem()
{
    m_curtok = tokenize(m_tokstr);
    if ((m_curtok != TOK_IDENT) && (m_curtok != TOK_HASH))
    {
        std::stringstream ss;
        ss << "Expected identifier in layer item but got " << m_tokstr;
        error(ss.str());
        return false;
    }
    if (m_tokstr == "PITCH")
    {
        return parseLayerPitch();   
    }
    else if (m_tokstr == "SPACING")
    {
        return parseLayerSpacing();
    }    
    else if (m_tokstr == "OFFSET")
    {
        return parseLayerOffset();
    }
    else if (m_tokstr == "TYPE")
    {
        return parseLayerType();
    }    
    else if (m_tokstr == "DIRECTION")
    {
        return parseLayerDirection();
    }
    else if (m_tokstr == "WIDTH")
    {
        return parseLayerWidth();
    }    
    else if (m_tokstr == "MAXWIDTH")
    {
        return parseLayerMaxWidth();
    }
    else if (m_tokstr == "MINWIDTH")
    {
        return parseLayerMinWidth();
    }
    else if (m_tokstr == "END")
    {
        return true;
    }
    else
    {
        // eat everything on the line
        while((m_curtok != TOK_EOL) && (m_curtok != TOK_EOF))
        {
            m_curtok = tokenize(m_tokstr);
        }
        if (m_curtok == TOK_EOF)
        {
            error("Unexpected end of file");
            return false;
        }
    }

    return true;
}

bool LEFParser::parseLayerSpacing()
{
    // SPACING <number> ;
    // or 
    // SPACING <number> RANGE <number> <number> ;
    // or (LEF version )
    // SPACING 0.090000 ENDOFLINE 0.090000 WITHIN 0.025000 ;

    std::string pitch;
    m_curtok = tokenize(m_tokstr);
    if (m_curtok != TOK_NUMBER)
    {
        error("Expected a number in layer spacing\n");
        return false;    
    }

    bool ok;
    int64_t pitchd = flt2int(m_tokstr, ok);

    onLayerSpacing(pitchd);

    // expect ; or RANGE
    m_curtok = tokenize(m_tokstr);
    if (m_tokstr == "RANGE")
    {
        parseLayerSpacingRange();
        m_curtok = tokenize(m_tokstr);
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

    if (m_curtok != TOK_SEMICOL)
    {
        error("Expected a semicolon in layer spacing\n");
        return false;
    }

    // expect EOL
    m_curtok = tokenize(m_tokstr);
    if (m_curtok != TOK_EOL)
    {
        error("Expected an EOL\n");
        return false;
    }

    return true;    
}

bool LEFParser::parseLayerSpacingRange()
{
    // expect: number number
    m_curtok = tokenize(m_tokstr);

    if (m_curtok != TOK_NUMBER)
    {
        error("Expected number after RANGE\n");
        return false;
    }

    bool ok;
    int64_t value1 = flt2int(m_tokstr, ok);

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

bool LEFParser::parseLayerPitch()
{
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

    // expect ; 
    
    if (m_curtok != TOK_SEMICOL)
    {
        error("Expected a semicolon in layer pitch\n");
        return false;
    }

    // expect EOL
    m_curtok = tokenize(m_tokstr);
    if (m_curtok != TOK_EOL)
    {
        error("Expected an EOL\n");
        return false;
    }

    bool ok;
    int64_t pitchd  = flt2int(pitch, ok);
    if (dualarg)
    {
        int64_t pitchd2 = flt2int(pitch2, ok);
        onLayerPitch(pitchd, pitchd2);
    }
    else
    {   
        onLayerPitch(pitchd);
    }
    

    

    return true;    
}


bool LEFParser::parseLayerOffset()
{
    std::string offset;
    m_curtok = tokenize(m_tokstr);
    if (m_curtok != TOK_NUMBER)
    {
        error("Expected a number in layer offset\n");
        return false;    
    }

    offset = m_tokstr;

    // expect ; 
    m_curtok = tokenize(m_tokstr);
    if (m_curtok != TOK_SEMICOL)
    {
        error("Expected a semicolon in layer offset\n");
        return false;
    }

    // expect EOL
    m_curtok = tokenize(m_tokstr);
    if (m_curtok != TOK_EOL)
    {
        error("Expected an EOL\n");
        return false;
    }

    bool ok;
    int64_t offsetd = flt2int(offset, ok);

    onLayerOffset(offsetd);

    return true;    
}

bool LEFParser::parseLayerType()
{
    std::string layerType;
    m_curtok = tokenize(m_tokstr);
    if (m_curtok != TOK_IDENT)
    {
        error("Expected a string in layer type\n");
        return false;    
    }

    layerType = m_tokstr;

    // expect ; 
    m_curtok = tokenize(m_tokstr);
    if (m_curtok != TOK_SEMICOL)
    {
        error("Expected a semicolon in layer type\n");
        return false;
    }

    // expect EOL
    m_curtok = tokenize(m_tokstr);
    if (m_curtok != TOK_EOL)
    {
        error("Expected an EOL\n");
        return false;
    }

    onLayerType(layerType);

    return true;    
}

bool LEFParser::parseLayerWidth()
{
    std::string width;
    m_curtok = tokenize(m_tokstr);
    if (m_curtok != TOK_NUMBER)
    {
        error("Expected a number in layer width\n");
        return false;    
    }

    width = m_tokstr;

    // expect ; 
    m_curtok = tokenize(m_tokstr);
    if (m_curtok != TOK_SEMICOL)
    {
        error("Expected a semicolon in layer width\n");
        return false;
    }

    // expect EOL
    m_curtok = tokenize(m_tokstr);
    if (m_curtok != TOK_EOL)
    {
        error("Expected an EOL\n");
        return false;
    }

    bool ok;
    int64_t widthd = flt2int(width, ok);

    onLayerWidth(widthd);

    return true;    
}

bool LEFParser::parseLayerMaxWidth()
{
    std::string maxwidth;
    m_curtok = tokenize(m_tokstr);
    if (m_curtok != TOK_NUMBER)
    {
        error("Expected a number in layer max width\n");
        return false;    
    }

    maxwidth = m_tokstr;

    // expect ; 
    m_curtok = tokenize(m_tokstr);
    if (m_curtok != TOK_SEMICOL)
    {
        error("Expected a semicolon in layer max width\n");
        return false;
    }

    // expect EOL
    m_curtok = tokenize(m_tokstr);
    if (m_curtok != TOK_EOL)
    {
        error("Expected an EOL\n");
        return false;
    }

    bool ok;
    int64_t maxwidthd = flt2int(maxwidth, ok);

    onLayerMaxWidth(maxwidthd);

    return true;
}


bool LEFParser::parseLayerMinWidth()
{
    std::string minwidth;
    m_curtok = tokenize(m_tokstr);
    if (m_curtok != TOK_NUMBER)
    {
        error("Expected a number in layer min width\n");
        return false;    
    }

    minwidth = m_tokstr;

    // expect ; 
    m_curtok = tokenize(m_tokstr);
    if (m_curtok != TOK_SEMICOL)
    {
        error("Expected a semicolon in layer min width\n");
        return false;
    }

    // expect EOL
    m_curtok = tokenize(m_tokstr);
    if (m_curtok != TOK_EOL)
    {
        error("Expected an EOL\n");
        return false;
    }

    bool ok;
    int64_t minwidthd = flt2int(minwidth, ok);

    onLayerMinWidth(minwidthd);

    return true;
}


bool LEFParser::parseLayerDirection()
{
    std::string direction;
    m_curtok = tokenize(m_tokstr);
    if (m_curtok != TOK_IDENT)
    {
        error("Expected a string in layer direction\n");
        return false;    
    }

    direction = m_tokstr;

    // expect ; 
    m_curtok = tokenize(m_tokstr);
    if (m_curtok != TOK_SEMICOL)
    {
        error("Expected a semicolon in layer direction\n");
        return false;
    }

    // expect EOL
    m_curtok = tokenize(m_tokstr);
    if (m_curtok != TOK_EOL)
    {
        error("Expected an EOL\n");
        return false;
    }

    onLayerDirection(direction);

    return true;  
}

bool LEFParser::parseVia()
{
    // VIA <vianame> ...
    // keep on reading tokens until we
    // find END <vianame>

    m_curtok = tokenize(m_tokstr);
    std::string viaName;

    if (m_curtok != TOK_IDENT)
    {
        error("Expected identifier in via name\n");
        return false;
    }

    viaName = m_tokstr;

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
    } while(m_tokstr == viaName);
    
    m_curtok = tokenize(m_tokstr);
    if (m_curtok != TOK_EOL)
    {
        error("Expected EOL after END <vianame>\n");
        return false;
    }

    return true;
}

bool LEFParser::parseViaRule()
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
    } while(m_tokstr == viaRuleName);
    
    m_curtok = tokenize(m_tokstr);
    if (m_curtok != TOK_EOL)
    {
        error("Expected EOL after END <viarule name>\n");
        return false;
    }

    return true;
}

bool LEFParser::parseUnits()
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

    while(1)
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
                    double micronsd;
                    try
                    {
                        micronsd = std::stod(m_tokstr);
                    }
                    catch(const std::invalid_argument& ia)
                    {
                        error(ia.what());
                        return false;
                    }

                    m_dBMicrons = micronsd;

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
            else
            {
                // read until EOL
                while ((m_curtok != TOK_EOL) && (m_curtok != TOK_EOF))
                {
                    m_curtok = tokenize(m_tokstr);
                }
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

bool LEFParser::parsePropertyDefintions()
{
    // basically, eat everything until
    // we encounter END PROPERTYDEFINTIONS EOL

    while(1)
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
        else if (m_curtok == TOK_EOF)
        {
            error("Unexpected end of liberty file\n");
            return false;
        }
    }
}

bool LEFParser::parseManufacturingGrid()
{
    m_curtok = tokenize(m_tokstr);
    if (m_curtok != TOK_NUMBER)
    {
        error("Expected a number after MANUFACTURINGGRID\n");
        return false;
    }

    // manufacturing grid is _always_ specified in microns
    double value;
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
#endif
