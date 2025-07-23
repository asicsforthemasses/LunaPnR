// SPDX-FileCopyrightText: 2021-2025 Niels Moseley <asicsforthemasses@gmail.com>
//
// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <list>
#include <vector>
#include <array>
#include <string>
#include <iostream>
#include <span>
#include "common/logging.h"

namespace LunaCore::Padring
{

class ConfigReader
{
public:
    ConfigReader(std::istream &is) : m_is(is) {}

    virtual ~ConfigReader() = default;

    enum class Token_t
    {
        END,
        IDENT,
        STRING,
        NUMBER,
        MINUS,
        LBRACKET,
        RBRACKET,
        LPAREN,
        RPAREN,
        HASH,
        SEMICOL,
        EOL,
        ERR
    };

    bool parse();

    /** callback for a corner */
    virtual void onCorner(
        const std::string &instance,
        const std::string &location)
    {
        std::cout << "CORNER " << instance << " " << location << "\n";
    }

    /** callback for a pad
     *  location is one of N,S,W,E
     *  if flipped == true, the (unplaced/unrotated) cell is flipped along the y axis.
    */
    virtual void onPad(
        const std::string &instance,
        const std::string &location,
        bool flipped)
    {
        std::cout << "PAD " << instance << " " << location << "\n";
    }

    /** callback for grid spacing in microns */
    virtual void onFiller(const std::string &fillerName)
    {
        std::cout << "Filler prefix:" << fillerName << "\n";
    }

    /** callback for space in microns */
    virtual void onSpace(double space)
    {
        std::cout << "Space " << space << "\n";
    }

    /** callback for offset in microns */
    virtual void onOffset(double offset)
    {
        std::cout << "Offset " << offset << "\n";
    }

    /** callback for offset in microns */
    virtual void onDefaultPadOrientation(const ChipDB::Orientation &orientation)
    {
        std::cout << "Default pad orientation " << orientation << "\n";
    }

    /** return the number of pad cells (excluding corners) */
    constexpr uint32_t getPadCellCount() const noexcept
    {
        return m_padCount;
    }


protected:
    constexpr bool isWhitespace(char c) const
    {
        return ((c==' ') || (c == '\t'));
    }

    constexpr bool isAlpha(char c) const
    {
        if (((c >= 'A') && (c <= 'Z')) || ((c >= 'a') && (c <= 'z')))
            return true;

        if ((c == '_') || (c == '!'))
            return true;

        return false;
    }

    constexpr bool isDigit(char c) const
    {
        return ((c >= '0') && (c <= '9'));
    }

    constexpr bool isAlphaNumeric(char c) const
    {
        return (isAlpha(c) || isDigit(c));
    }

    constexpr bool isSpecialIdentChar(char c) const
    {
        if ((c == '[') || (c == ']') ||
            (c == '<') || (c == '>') ||
            (c == '/') || (c == '\\') ||
            (c == '.'))
        {
            return true;
        }
        return false;
    }

    bool inArray(const std::string &value, std::span<std::string> span);

    bool parsePad();
    bool parseCorner();
    bool parseSpace();
    //bool parseOffset();
    //bool parseFiller();
    bool parseDefaultPadOrientation();

    constexpr bool match(char c) noexcept
    {
        if (m_tokchar == c)
        {
            m_tokchar = m_is.get();
            return true;
        }
        return false;
    }

    Token_t      tokenize(std::string &tokstr);
    char         m_tokchar{' '};

    void error(const std::string_view fmt, ...);
    void warning(const std::string_view fmt, ...);
    void info(const std::string_view fmt, ...);

    std::istream &m_is;
    uint32_t      m_lineNum{1};
    uint32_t      m_padCount{0};    ///< number of pad cells excluding corners
};

};