// SPDX-FileCopyrightText: 2021-2024 Niels Moseley <asicsforthemasses@gmail.com>
//
// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <list>
#include <vector>
#include <string>
#include <iostream>
#include <optional>
#include <regex>

#include "database/database.h"

namespace ChipDB::DEF
{

/** reads a DEF stream and generates callbacks for every relevant
    item.

    all coordinates, sizes are in nanometers
*/
class Parser
{
public:
    Parser() :
        m_curtok(TOK_ERR),
        m_src(nullptr),
        m_idx(0),
        m_lineNum(0),
        m_col(0),
        m_dBMicrons(0),
        m_resUnit(1.0),     // 1 ohm default
        m_capUnit(1.0e-12)  // 1 pF default
        {}

    virtual ~Parser() {}

    enum token_t
    {
        TOK_EOF,
        TOK_IDENT,
        TOK_STRING,
        TOK_NUMBER,
        TOK_MINUS,
        TOK_PLUS,
        TOK_LBRACKET,
        TOK_RBRACKET,
        TOK_LPAREN,
        TOK_RPAREN,
        TOK_HASH,
        TOK_SEMICOL,
        TOK_STAR,
        TOK_EOL,
        TOK_ERR
    };

    bool parse(const std::string &defstring);

    /** callback for each DESIGN statement */
    virtual void onDesign(const std::string &designName) {}

    /** callback for each END DESIGN statement */
    virtual void onEndDesign(const std::string &designName) {}

    /** callback for each component */
    virtual void onComponent(const std::string &insName,
            const std::string &archetype) {};

    /** callback for each component position */
    virtual void onComponentPlacement(const ChipDB::Coord64 &pos,
        const ChipDB::PlacementInfo placement,
        const ChipDB::Orientation orient) {};

    virtual void onEndParse() {};

protected:
    static bool isWhitespace(char c);
    static bool isAlpha(char c);
    static bool isDigit(char c);
    static bool isAlphaNumeric(char c);
    static bool isExtendedAlphaNumeric(char c);

    bool parseDesign();
    bool parseComponents();
    bool parseComponent();

    bool parsePlaced();
    bool parseUnplaced();
    bool parseFixed();

    bool parsePropertyDefinitions();
    bool parseVias();

    bool parseUntilEnd(const std::string &postfix);

    std::optional<ChipDB::Coord64> parsePoint();

    bool skipUntilSemicolon();
    bool skipUntilEOL();
    bool skipUntilEOLorSemicolon();

#if 0

    bool parseMacro();
    bool parseClass();
    bool parseOrigin();
    bool parseForeign();
    bool parseSize();
    bool parseSymmetry();
    bool parseMacroSite();
    bool parsePin();
    bool parseDirection();
    bool parseUse();
    bool parseManufacturingGrid();

    bool parsePort();
    bool parsePortLayer();
    bool parsePortLayerItem();
    bool parseRect();
    bool parsePolygon();

    bool parseLayer();
    bool parseLayerItem();
    bool parseLayerType();
    bool parseLayerPitch();
    bool parseLayerSpacing();
    bool parseLayerSpacingRange();
    bool parseLayerWidth();
    bool parseLayerMaxWidth();
    bool parseLayerMinWidth();
    bool parseLayerDirection();
    bool parseLayerOffset();

    bool parseLayerResistance();
    bool parseLayerCapacitance();
    bool parseLayerEdgeCapacitance();

    bool parseLayerMinArea();
    bool parseLayerThickness();

    bool parseLayerSpacingTable();

    bool parseSiteItem();
    bool parseSiteClass();
    bool parseSiteSymmetry();
    bool parseSiteSize();

    bool parseVia();
    bool parseViaRule();

    bool parseObstruction();
    bool parseObstructionLayer();

    bool parseUnits();

    bool parsePropertyDefintions();

    bool expectSemicolonAndEOL();

    bool skipUntilSemicolon();
#endif


    token_t tokenize(std::string &tokstr);

    token_t     m_curtok;
    std::string m_tokstr;

    void error(const char *errstr) const;
    void error(const std::string &errstr) const;

    bool match(char c);
    void advance();
    char peek() const;
    bool atEnd() const;

    int64_t flt2int(const std::string &value, bool &ok);  ///< convert LEF/DEF values to nanometers

    const std::string *m_src;
    uint32_t    m_idx;
    uint32_t    m_lineNum;
    uint32_t    m_col;

    ChipDB::Coord64 m_lastPoint{0,0};    ///< last point, to implement ( * * ) ... ugh.

    float m_dBMicrons;  ///< database units to convert value to microns, default = 100
    double m_resUnit;   ///< unit of resistance, default 1 ohm?
    double m_capUnit;   ///< unit of capacitance, default 1 pF?
};

}; //DEF namespace
