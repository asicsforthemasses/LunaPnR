/*
    Copyright (c) 2020, 2021 Niels Moseley <n.a.moseley@moseleyinstruments.com>

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

#include <list>
#include <vector>
#include <string>
#include <iostream>
#include <regex>

#include "common/dbtypes.h"

namespace ChipDB::LEF
{

/** reads a LEF stream and generates callbacks for every relevant
    item, such as .input .output etc.

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
        TOK_LBRACKET,
        TOK_RBRACKET,
        TOK_LPAREN,
        TOK_RPAREN,
        TOK_HASH,
        TOK_SEMICOL,
        TOK_EOL,
        TOK_ERR
    };

    bool parse(const std::string &lefstring);

    /** callback for each LEF macro */
    virtual void onMacro(const std::string &macroName) {}

    /** callback for each LEF end macro */
    virtual void onEndMacro(const std::string &macroName) {}

    /** callback for CLASS within a macro */
    virtual void onClass(const std::string &className) {}

    /** callback for CLASS and SUBCLASS within a macro */
    virtual void onClass(const std::string &className,
        const std::string &subclass) {}

    /** callback for ORIGIN within a macro */
    virtual void onOrigin(int64_t x, int64_t y) {}

    /** callback for ORIGIN within a macro */
    virtual void onForeign(const std::string &foreignName, int64_t x, int64_t y) {}

    /** callback for SIZE within a macro */
    virtual void onSize(int64_t sx, int64_t sy) {}

    /** callback for SYMMETRY within a macro */
    //virtual void onSymmetry(const SymmetryType &symmetry) {}

    /** callback for SITE within a macro */
    virtual void onMacroSite(const std::string &site) {}

    /** callback for PIN within a macro */
    virtual void onPin(const std::string &pinName) {}

    /** callback for PIN direction */
    virtual void onPinDirection(const std::string &direction) {}

    /** callback for PIN use */
    virtual void onPinUse(const std::string &use) {}

    /** callback for port/layer rectangle */
    virtual void onEndPin(const std::string &pinName) {}

    /** callback for obstruction defintion */
    virtual void onObstruction() {}

    /** callback for each layer of the obstruction,
     *  followed by onObstructionLayer and onRect calls.
     */
    virtual void onObstructionLayer(const std::string &layerName) {}

    /** callback for end obstruction defintion */
    virtual void OnEndObstruction() {}

    /** callback when done parsing */
    virtual void onEndParse() {}

    /** callback for layer */
    virtual void onLayer(const std::string &layerName) {}

    /** callback for layer */
    virtual void onEndLayer(const std::string &layerName) {}

    /** callback for layer type */
    virtual void onLayerType(const std::string &layerType) {}

    /** callback for layer pitch in x and y direction */
    virtual void onLayerPitch(int64_t xpitch, int64_t ypitch) {}

    /** callback for layer spacing */
    virtual void onLayerSpacing(int64_t spacing) {}

    /** callback for layer spacing range specification */
    virtual void onLayerSpacingRange(int64_t value1, int64_t value2) {}

    /** callback for layer spaceing range influence (onLayerSpacingRange is called first) */
    virtual void onLayerSpacingRangeInfluence(int64_t influence) {}

    /** callback for layer offset (x and y offsets) */    
    virtual void onLayerOffset(int64_t x_offset, int64_t y_offset) {}

    /** callback for layer routing direction */
    virtual void onLayerDirection(const std::string &direction) {}

    /** callback for layer trace width */
    virtual void onLayerWidth(int64_t width) {}

    /** callback for layer trace max width */    
    virtual void onLayerMaxWidth(int64_t maxWidth) {}

    /** callback for layer trace min width */    
    virtual void onLayerMinWidth(int64_t minWidth) {}

    /** callback for layer resistance per square (in ohms) */
    virtual void onLayerResistancePerSq(double ohms) {}

    /** callback for layer capacitance per micron^2 (in farad) */
    virtual void onLayerCapacitancePerSq(double farads) {}

    /** callback for layer edge capacitance per micron (in farad) */
    virtual void onLayerEdgeCapacitance(double farads) {}

    /** callback for layer thickness */
    virtual void onLayerThickness(double thickness) {}

    /** callback for layer minimum area in microns^2 */
    virtual void onLayerMinArea(double minArea) {}

    /** callback for port */
    virtual void onPort() {}

    /** callback for port layer */
    virtual void onPortLayer(const std::string &name) {}

    /** callback for end port */
    virtual void onEndPort() {}

    /** callback for port/layer rectangle */
    virtual void onRect(int64_t x1, int64_t y1, int64_t x2, int64_t y2) {}

    /** Callback for a polygon (al least three points) 
     *  all angles must be 180,90 or 45 degrees.
     *  The last point is equal to the first point but
     *  the last point is not stored in the points array.
    */
    virtual void onPolygon(const std::vector<Coord64> &points) {}

    virtual void onSite(const std::string &siteName) {}

    virtual void onEndSite(const std::string &siteName) {}

    virtual void onSiteClass(const std::string &siteClass) {}

    //virtual void onSiteSymmetry(const SymmetryType &symmetry) {}

    virtual void onSiteSize(int64_t x, int64_t y) {}

    /** callback for units database microns */
    virtual void onDatabaseUnitsMicrons(int64_t unitsPerMicron) {}

    /** callback for manufacturing grid */
    virtual void onManufacturingGrid(int64_t value) {}

protected:
    bool isWhitespace(char c) const;
    bool isAlpha(char c) const;
    bool isDigit(char c) const;
    bool isAlphaNumeric(char c) const;
    bool isExtendedAlphaNumeric(char c) const;

    bool parseSite();

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

    token_t tokenize(std::string &tokstr);

    token_t     m_curtok;
    std::string m_tokstr;

    void error(const std::string &errstr);

    bool match(char c);
    void advance();
    char peek() const;
    bool atEnd() const;
    
    int64_t flt2int(const std::string &value, bool &ok);  ///< convert LEF/DEF values to nanometers

    const std::string *m_src;    
    uint32_t    m_idx;
    uint32_t    m_lineNum;
    uint32_t    m_col;

    float m_dBMicrons;  ///< database units to convert value to microns, default = 100
    double m_resUnit;   ///< unit of resistance, default 1 ohm?
    double m_capUnit;   ///< unit of capacitance, default 1 pF?
};

}; //LEF namespace



