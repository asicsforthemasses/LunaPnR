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

#pragma once

#include<stdint.h>
#include<list>
#include<iostream>
#include<vector>
#include<string>
#include<iostream>

namespace ChipDB::Liberty {

/** reads a Liberty stream and generates callbacks for every relevant
    item.

    all coordinates, sizes are in nanometers.
*/
class Parser
{
public:
    Parser() : 
        m_curtok(TOK_ERR), 
        m_src(nullptr),
        m_idx(0),
        m_lineNum(0),
        m_col(0)
         {}
    
    virtual ~Parser() {}

    enum token_t : int32_t
    {
        TOK_EOF,
        TOK_COMMENT,
        TOK_IDENT,
        TOK_STRING,
        TOK_NUMBER,
        TOK_MINUS,
        TOK_PLUS,
        TOK_LBRACKET,
        TOK_RBRACKET,
        TOK_LCURLY,
        TOK_RCURLY,
        TOK_LPAREN,
        TOK_RPAREN,
        TOK_HASH,
        TOK_SEMICOL,
        TOK_COLON,
        TOK_COMMA,
        TOK_SLASH,
        TOK_BSLASH,
        TOK_STAR,
        TOK_EOL,
        TOK_ERR
    };

    bool parse(const std::string &libertyString);

    /** Called for groups without a name/parameter */
    virtual void onGroup(const std::string &group) {}

    /** called for groups with a name/parameter, such as library(libname).
     *  group = "library", name = libname.
    */
    virtual void onGroup(const std::string &group, const std::string &name) {}

    /** Called for a simple attribute. name : value */
    virtual void onSimpleAttribute(const std::string &name, const std::string &value) {}

    /** Called for complex attributes, such as timing tables */
    virtual void onComplexAttribute(const std::string &attrname, const std::vector<std::string> &list) {}
    
    /** Called at the end of every group */
    virtual void onEndGroup() {}

    /** Called at the end of parsing */
    virtual void onEndParse() {}

protected:
    bool isWhitespace(char c) const;
    bool isAlpha(char c) const;
    bool isDigit(char c) const;
    bool isAlphaNumeric(char c) const;
    bool isExtendedAlphaNumeric(char c) const;

    bool parseStatement();
    bool parseList(std::vector<std::string> &list);
    bool parseDefine();
    bool parseGroup();
    bool parseGroupOrComplexAttribute(const std::string &group);
    bool parseSimpleAttribute(const std::string &name);

    /** tokenize the next part and return
     *  the string in tokstr and the
     *  actual token as return value
    */
    token_t tokenize(std::string &tokstr);

    void error(const std::string &errstr);

    /** accept the current token if it is equal
     *  to 'tok', advance to next token, return true, 
     * else return false.
    */
    bool acceptToken(const token_t tok);
    void advanceToken();
    token_t peekToken() const
    {
        return m_curtok;
    }
    
    // lexer functions
    token_t         m_curtok;
    std::string     m_tokstr;

    bool match(char c);
    void advance();
    char peek() const;
    bool atEnd() const;
    
    const std::string *m_src;
    uint32_t    m_idx;
    uint32_t    m_lineNum;
    uint32_t    m_col;
};

}; // namespace

