// SPDX-FileCopyrightText: 2021-2025 Niels Moseley <asicsforthemasses@gmail.com>
//
// SPDX-License-Identifier: GPL-3.0-only

#include <algorithm>
#include "veriloglexer.h"
#include "common/logging.h"

using namespace ChipDB::Verilog;

bool Lexer::match(char c)
{
    if (peek() == c)
    {
        m_idx++;
        return true;
    }

    return false;
}

/** peek at the next character */
char Lexer::peek() const
{
    if (atEnd())
        return 0;

    return m_src->at(m_idx);
}


void Lexer::advance()
{
    if (!atEnd())
    {
        m_col++;
        m_idx++;
    }
}

/** returns true if we're at the end of the source */
bool Lexer::atEnd() const
{
    return !(m_idx < m_src->size());
}

bool Lexer::isDigit(char c) const
{
    return (c >= '0') && (c <= '9');
}

bool Lexer::isAlpha(char c) const
{
    return ((c >= 'a') && (c <= 'z')) ||
        ((c >= 'A') && (c <= 'Z')) ||
        (c == '_');
}

bool Lexer::isHex(char c) const
{
    return ((c >= 'a') && (c <= 'f')) ||
        ((c >= 'A') && (c <= 'F')) || isDigit(c);
}

bool Lexer::isBin(char c) const
{
    if ((c == '0') || (c == '1'))
        return true;

    return false;
}

bool Lexer::isWhitespace(char c) const
{
    return (c == '\r') || (c == '\n') || (c == '\t') || (c== ' ');
}

bool Lexer::execute(const std::string &src, std::vector<token> &tokens)
{
    std::string tmpstr;
    token token;

    m_col = 1;
    m_line = 1;
    m_idx = 0;
    m_src = &src;

    m_state = S_IDLE;
    while(!atEnd())
    {
        if (m_state == S_IDLE)
        {
            token.m_value.clear();
            char c = peek();

            // skip white space
            if ((c == ' ') || (c == '\t'))
            {
                advance();
                continue;
            }

            token.m_col = m_col;
            token.m_line = m_line;

            // read numbers
            if (isDigit(c))
            {
                token.m_tok = TOK_INTEGER;
                token.m_value += c;
                advance();
                m_state = S_INTEGER;
                continue;
            }

            // read identifiers
            if (isAlpha(c))
            {
                token.m_tok = TOK_IDENT;
                token.m_value += c;
                advance();
                m_state = S_IDENT;
                continue;
            }

            switch(c)
            {
            case 10:
                advance();
                match(13);  // skip additional CR
                m_col = 1;
                m_line++;
                break;
            case 13:
                advance();
                match(10);  // skip additional LF
                m_col = 1;
                m_line++;
                break;
            case '(':
                advance();
                if (match('*'))
                {
                    // start of attribute
                    token.m_tok = TOK_ATTRIBUTE;
                    m_state = S_ATTRIBUTE;
                }
                else
                {
                    token.m_tok = TOK_LPAREN;
                    tokens.push_back(token);
                }
                break;
            case ')':
                token.m_tok = TOK_RPAREN;
                tokens.push_back(token);
                advance();
                break;
            case '[':
                token.m_tok = TOK_LBRACKET;
                tokens.push_back(token);
                advance();
                break;
            case ']':
                token.m_tok = TOK_RBRACKET;
                tokens.push_back(token);
                advance();
                break;
            case '{':
                token.m_tok = TOK_LCURLY;
                tokens.push_back(token);
                advance();
                break;
            case '}':
                token.m_tok = TOK_RCURLY;
                tokens.push_back(token);
                advance();
                break;
            case '=':
                token.m_tok = TOK_EQUAL;
                tokens.push_back(token);
                advance();
                break;
            case ',':
                token.m_tok = TOK_COMMA;
                tokens.push_back(token);
                advance();
                break;
            case '.':
                token.m_tok = TOK_PERIOD;
                tokens.push_back(token);
                advance();
                break;
            case ';':
                token.m_tok = TOK_SEMICOL;
                tokens.push_back(token);
                advance();
                break;
            case ':':
                token.m_tok = TOK_COLON;
                tokens.push_back(token);
                advance();
                break;
            case '\\':  // start of escaped identifier
                token.m_tok = TOK_IDENT;
                token.m_value.clear();
                advance();
                m_state = S_ESCAPEDIDENT;
                break;
            case '/':
                advance();
                if (match('*'))
                {
                    m_state = S_COMMENT;
                }
                else if (match('/'))
                {
                    m_state = S_LINECOMMENT;
                }
                else
                {
                    token.m_tok = TOK_SLASH;
                    tokens.push_back(token);
                }
                break;
            default:
                Logging::logError("Skipping %c (%d) on line %d\n", c,
                    static_cast<uint32_t>(c), m_line);
                advance();
                break;
            }
        }
        else if (m_state == S_ATTRIBUTE)
        {
            if (match('*'))
            {
                if (match(')'))
                {
                    tokens.push_back(token);
                    m_state = S_IDLE;
                }
            }
            else
            {
                token.m_value += peek();
                advance();
            }
        }
        else if (m_state == S_IDENT)
        {
            char c = peek();
            while (isDigit(c) || isAlpha(c))
            {
                token.m_value += c;
                advance();
                c = peek();
            }

            // check for keywords
            tmpstr = token.m_value;
            std::transform(tmpstr.begin(), tmpstr.end(), tmpstr.begin(), ::toupper);

            if (tmpstr == "WIRE")
            {
                token.m_tok = TOK_WIRE;
            }
            else if (tmpstr == "MODULE")
            {
                token.m_tok = TOK_MODULE;
            }
            else if (tmpstr == "ENDMODULE")
            {
                token.m_tok = TOK_ENDMODULE;
            }
            else if (tmpstr == "ASSIGN")
            {
                token.m_tok = TOK_ASSIGN;
            }
            else if (tmpstr == "INPUT")
            {
                token.m_tok = TOK_INPUT;
            }
            else if (tmpstr == "OUTPUT")
            {
                token.m_tok = TOK_OUTPUT;
            }

            tokens.push_back(token);
            m_state = S_IDLE;
        }
        else if (m_state == S_ESCAPEDIDENT)
        {
            char c = peek();
            while (!isWhitespace(c))
            {
                token.m_value += c;
                advance();
                c = peek();
            }
            tokens.push_back(token);
            m_state = S_IDLE;
        }
        else if (m_state == S_INTEGER)
        {
            char c = peek();
            while (isDigit(c))
            {
                token.m_value += c;
                advance();
                c = peek();
            }

            // check if this is a hex,dec or bin constant
            if (c == '\'')
            {
                advance();
                c = peek();
                if (c == 'h')
                {
                    m_bitwidth = std::stod(token.m_value);
                    token.m_value.clear();
                    advance();
                    m_state = S_HEXINTEGER;
                    continue;
                }
                else if (c == 'd')
                {
                    m_bitwidth = std::stod(token.m_value);
                    token.m_value.clear();
                    advance();
                    m_state = S_DECINTEGER;
                    continue;
                }
                else if (c == 'b')
                {
                    m_bitwidth = std::stod(token.m_value);
                    token.m_value.clear();
                    advance();
                    m_state = S_BININTEGER;
                    continue;
                }
                else
                {
                    Logging::logError("Expected 'd' or 'h' or 'b' after bit width specification.\n");
                    return false;
                }
            }
            else
            {
                // regular integer, so emit!
                tokens.push_back(token);
                m_state = S_IDLE;
            }
        }
        else if (m_state == S_HEXINTEGER)
        {
            char c = peek();
            while (isHex(c) || c=='X')
            {
                // replace all don't care's with zeros.
                if (c == 'X') c = '0';

                token.m_value += c;
                advance();
                c = peek();
            }
            tokens.push_back(token);
            m_state = S_IDLE;
        }
        else if (m_state == S_DECINTEGER)
        {
            char c = peek();
            while (isDigit(c))
            {
                token.m_value += c;
                advance();
                c = peek();
            }
            tokens.push_back(token);
            m_state = S_IDLE;
        }
        else if (m_state == S_BININTEGER)
        {
            char c = peek();
            while (isBin(c))
            {
                token.m_value += c;
                advance();
                c = peek();
            }
            tokens.push_back(token);
            m_state = S_IDLE;
        }
        else if (m_state == S_COMMENT)
        {
            if (match('*'))
            {
                if (match('/'))
                {
                    m_state = S_IDLE;
                }
            }
            else
            {
                // still in comment, next char..
                advance();
            }
        }
        else if (m_state == S_LINECOMMENT)
        {
            // wait for EOL.
            char c = peek();
            if ((c != 10) && (c != 13) && (!atEnd()))
            {
                advance();
            }
            else
            {
                m_line++;
                m_col = 1;
                m_state = S_IDLE;
            }
        }
        else
        {
            Logging::logError("Error: incorrect state\n");
            return false;
        }
    }

    Logging::logVerbose("Lexer processed %d lines\n", m_line);

    return true;
}