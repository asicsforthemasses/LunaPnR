// SPDX-FileCopyrightText: 2021-2025 Niels Moseley <asicsforthemasses@gmail.com>
//
// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include "lefparser.h"
#include "database/database.h"

namespace ChipDB::LEF
{

class ReaderImpl : public Parser
{
public:
    ReaderImpl(Design &design);

    virtual void onMacro(const std::string &macroName) override;

    /** callback for each LEF end macro */
    virtual void onEndMacro(const std::string &macroName) override;

    /** callback for CLASS within a macro */
    virtual void onClass(const std::string &className) override;

    /** callback for CLASS and SUBCLASS within a macro */
    virtual void onClass(const std::string &className,
        const std::string &subclass) override;

    /** callback for ORIGIN within a macro */
    virtual void onOrigin(int64_t x, int64_t y) override {}

    /** callback for ORIGIN within a macro */
    virtual void onForeign(const std::string &foreignName, int64_t x, int64_t y) override {}

    /** callback for SIZE within a macro */
    virtual void onSize(int64_t sx, int64_t sy) override;

    /** callback for SYMMETRY within a macro */
    virtual void onSymmetry(const SymmetryFlags &symmetry) override;

    /** callback for SITE within a macro */
    virtual void onMacroSite(const std::string &site) override;

    /** callback for PIN within a macro */
    virtual void onPin(const std::string &pinName) override;

    /** callback for PIN direction */
    virtual void onPinDirection(const std::string &direction) override;

    /** callback for PIN use */
    virtual void onPinUse(const std::string &use) override;

    /** callback for port/layer rectangle */
    virtual void onEndPin(const std::string &pinName) override;

    /** callback for obstruction defintion */
    virtual void onObstruction() override;

    /** callback for each layer of the obstruction,
     *  followed by onObstructionLayer and onRect calls.
     */
    virtual void onObstructionLayer(const std::string &layerName) override;

    /** callback for end obstruction defintion */
    virtual void OnEndObstruction() override {}

    /** callback when done parsing */
    virtual void onEndParse() override {}

    /** callback for via */
    virtual void onVia(const std::string &viaName) override;

    /** callback for viarule */
    virtual void onViaRule(const std::string &viaRuleName) override;

    /** callback for layer */
    virtual void onLayer(const std::string &layerName) override;

    /** callback for layer */
    virtual void onEndLayer(const std::string &layerName) override;

    /** callback for layer type */
    virtual void onLayerType(const std::string &layerType) override;

    /** callback for layer pitch */
    virtual void onLayerPitch(int64_t pitchx, int64_t pitchy) override;

    /** callback for layer spacing */
    virtual void onLayerSpacing(int64_t spacing) override;

    /** callback for layer spacing range specification */
    virtual void onLayerSpacingRange(int64_t value1, int64_t value2) override;

    /** callback for layer offset */
    virtual void onLayerOffset(int64_t offsetx, int64_t offsety) override;

    /** callback for layer routing direction */
    virtual void onLayerDirection(const std::string &direction) override;

    /** callback for layer trace width */
    virtual void onLayerWidth(int64_t width) override;

    /** callback for layer trace max width */
    virtual void onLayerMaxWidth(int64_t maxWidth) override;

    /** callback for layer resistance per square (in ohms) */
    virtual void onLayerResistancePerSq(double ohms) override;

    /** callback for layer capacitance per micron^2 (in farad) */
    virtual void onLayerCapacitancePerSq(double farads) override;

    /** callback for layer edge capacitance per micron (in farad) */
    virtual void onLayerEdgeCapacitance(double farads) override;

    /** callback for layer thickness */
    virtual void onLayerThickness(double thickness) override;

    /** callback for layer minimum area in microns^2 */
    virtual void onLayerMinArea(double minArea) override;

    /** callback for port */
    virtual void onPort() override {}

    /** callback for port layer */
    virtual void onPortLayer(const std::string &name) override;

    /** callback for end port */
    virtual void onEndPort() override {}

    /** callback for port/layer rectangle */
    virtual void onRect(int64_t x1, int64_t y1, int64_t x2, int64_t y2) override;

    /** callback for port/layer rectangle */
    virtual void onPolygon(const std::vector<Coord64> &points) override;

    /** callback for units database microns */
    virtual void onDatabaseUnitsMicrons(int64_t unitsPerMicron) override {}

    /** callback for manufacturing grid */
    virtual void onManufacturingGrid(int64_t value) override;

    /** callback for SITE defintion */
    virtual void onSite(const std::string &site) override;

    virtual void onSiteSize(int64_t x, int64_t y) override;

    virtual void onSiteSymmetry(const SymmetryFlags &symmetry) override;

    virtual void onSiteClass(const std::string &siteClass) override;

    /** callback for SITE is done */
    virtual void onEndSite(const std::string &site) override;

protected:

    enum context_t
    {
        CONTEXT_PIN,
        CONTEXT_OBS
    } m_context;        ///< are we processing pins or obstructions?

    std::string m_pinUse;
    std::string m_pinDirection;

    std::shared_ptr<Cell>       m_curCell;
    std::shared_ptr<PinInfo>    m_curPinInfo;
    std::shared_ptr<LayerInfo>  m_curLayerInfo;
    std::shared_ptr<SiteInfo>   m_curSiteInfo;

    Design     &m_design;

    std::string m_activePinLayerName;
    std::string m_activeObsLayerName;
};

}; //namespace
