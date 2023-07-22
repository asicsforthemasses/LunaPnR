// SPDX-FileCopyrightText: 2021-2022,2023 Niels Moseley <asicsforthemasses@gmail.com>
//
// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <vector>
#include <string>
#include <stdint.h>

namespace ChipDB::Verilog
{

/// \cond HIDE_INTERNALS

class Lexer
{
public:
    enum tokenId
    {
        TOK_LPAREN,
        TOK_RPAREN,
        TOK_LBRACKET,
        TOK_RBRACKET,
        TOK_COMMA,
        TOK_SEMICOL,
        TOK_COLON,
        TOK_EQUAL,
        TOK_SLASH,
        TOK_BACKSLASH,
        TOK_LCURLY,
        TOK_RCURLY,
        TOK_PERIOD,
        TOK_ASSIGN,
        TOK_MODULE,
        TOK_ENDMODULE,
        TOK_INPUT,
        TOK_OUTPUT,
        TOK_WIRE,
        TOK_ATTRIBUTE,
        TOK_INTEGER,
        TOK_IDENT
    };

    struct token
    {
        tokenId     m_tok;
        std::string m_value;
        uint32_t    m_line;
        uint32_t    m_col;
    };

    /** tokenize the input source 
     *  returns true if no errors.
    */
    bool execute(const std::string &src, std::vector<token> &tokens);

protected:
    /** if character == c -> advance and return true,
     *  else return false
    */ 
    bool match(char c);

    /** peek at the next character */
    char peek() const;

    /** advance to the next character */
    void advance();

    /** returns true if we're at the end of the source */
    bool atEnd() const;

    bool isDigit(char c) const;
    bool isAlpha(char c) const;
    bool isHex(char c) const;
    bool isBin(char c) const;
    bool isWhitespace(char c) const;
    
    uint32_t m_idx;     ///< index of current character
    uint32_t m_line;    ///< current line number
    uint32_t m_col;     ///< current column number
    
    enum state_t 
    {
        S_IDLE,
        S_COMMENT,
        S_LINECOMMENT,
        S_ATTRIBUTE,
        S_INTEGER,
        S_IDENT,
        S_ESCAPEDIDENT,
        S_HEXINTEGER,
        S_DECINTEGER,
        S_BININTEGER
    };

    state_t m_state;

    uint32_t           m_bitwidth;  ///< number of bit in constants
    const std::string *m_src;       ///< source text
};

/// \endcond

}; // namespace
