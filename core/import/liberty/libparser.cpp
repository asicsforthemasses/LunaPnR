// SPDX-FileCopyrightText: 2021-2025 Niels Moseley <asicsforthemasses@gmail.com>
//
// SPDX-License-Identifier: GPL-3.0-only

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
            Logging::logError("Liberty parse error, current character = '%c' (0x%02X)\n", peek(), static_cast<uint32_t>(peek()));
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
    Logging::logError(ss.str());
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

