// SPDX-FileCopyrightText: 2021-2023 Niels Moseley <asicsforthemasses@gmail.com>
//
// SPDX-License-Identifier: GPL-3.0-only

#include <vector>
#include <array>
#include <string>
#include <memory>
#include "lefreaderimpl.h"
#include "design/design.h"
#include "common/logging.h"

using namespace ChipDB::LEF;

ReaderImpl::ReaderImpl(Design &design) 
    : m_design(design) 
{
    m_context = CONTEXT_PIN;

#if 0
    m_curCell       = nullptr;
    m_curPinInfo    = nullptr;
    m_curLayerInfo  = nullptr;
    m_curSiteInfo   = nullptr;
#endif
}

void ReaderImpl::onMacro(const std::string &macroName)
{
    // create a new cell/macro if necessary    
    auto newCellKeyObj = m_design.m_cellLib->createCell(macroName);
    if (!newCellKeyObj.isValid())
    {
        std::stringstream ss;
        ss << "Cannot create LEF MACRO " << macroName << " - perhaps it already exists?\n";
        Logging::doLog(Logging::LogType::ERROR, ss.str());
        m_curCell = nullptr;
    }
    else
    {
        Logging::doLog(Logging::LogType::VERBOSE,"LEF MACRO: %s\n", macroName.c_str());
        m_curCell = newCellKeyObj.ptr();
    }
}

void ReaderImpl::onEndMacro(const std::string &macroName)
{   
    m_curPinInfo.reset();
    m_curCell.reset();
}

void ReaderImpl::onVia(const std::string &viaName)
{
    Logging::doLog(Logging::LogType::VERBOSE,"LEF VIA: %s\n", viaName.c_str());
}

void ReaderImpl::onViaRule(const std::string &viaRuleName)
{
    Logging::doLog(Logging::LogType::VERBOSE,"LEF VIARULE: %s\n", viaRuleName.c_str());
}

void ReaderImpl::onSize(int64_t sx, int64_t sy)
{   
    const double nm2microns = 0.001;

    if (!m_curCell)
    {
        return;
    }

    // sx, sy are in nanometers
    m_curCell->m_size.m_x = sx;
    m_curCell->m_size.m_y = sy;
    m_curCell->m_area = (sx*nm2microns) * (sy*nm2microns);
}

void ReaderImpl::onMacroSite(const std::string &site)
{
    if (!m_curCell)
    {
        return;
    }

    m_curCell->m_site = site;
}

void ReaderImpl::onPin(const std::string &pinName)
{
    if (!m_curCell)
    {
        return;
    }

    // find already existing pin or create one.
    auto newOrExistingPin = m_curCell->createPin(pinName);
    if (newOrExistingPin.isValid())
    {
        m_curPinInfo = newOrExistingPin.ptr();
    }
    else
    {
        //FIXME: there really should be a warning here.
    }
    
    m_context = CONTEXT_PIN;
}

void ReaderImpl::onEndPin(const std::string &pinName)
{
    if (!m_curCell)
    {
        return;
    }

    if (!m_curPinInfo)
    {
        return;
    }

    // figure out the type of pin
    if (m_pinUse == "GROUND")
    {
        m_curPinInfo->m_iotype = IOType::GROUND;
    }
    else if (m_pinUse == "POWER")
    {
        m_curPinInfo->m_iotype = IOType::POWER;
    }
    else if (m_pinDirection == "INOUT")
    {
        m_curPinInfo->m_iotype = IOType::IO;
    }
    else if (m_pinDirection == "INPUT")
    {
        // Note: INPUT pins may later be promoted to 
        //       CLOCK type when the Liberty file has been
        //       read
        m_curPinInfo->m_iotype = IOType::INPUT;
    }
    else if (m_pinDirection == "OUTPUT")
    {
        m_curPinInfo->m_iotype = IOType::OUTPUT;
    }
    else if (m_pinDirection == "OUTPUT TRISTATE")
    {
        m_curPinInfo->m_iotype = IOType::OUTPUT_TRI;
    }    
    else
    {
        m_curPinInfo->m_iotype = IOType::UNKNOWN;
    }

    m_pinDirection.clear();
    m_pinUse.clear();
    m_curPinInfo.reset();
}

void ReaderImpl::onClass(const std::string &className)
{
    if (!m_curCell)
    {
        return;
    }
    
    std::string classNameUpper = toUpper(className);

    if (classNameUpper == "CORE")
    {
        m_curCell->m_class = CellClass::CORE;
    }
    else if (classNameUpper == "COVER")
    {
        m_curCell->m_class = CellClass::COVER;
    }
    else if (classNameUpper == "PAD")
    {
        m_curCell->m_class = CellClass::PAD;
    }
    else if (classNameUpper == "ENDCAP")
    {
        m_curCell->m_class = CellClass::ENDCAP;
    }
    else if (classNameUpper == "RING")
    {
        m_curCell->m_class = CellClass::RING;
    }
    else
    {
        std::stringstream ss;
        ss << "Unknown macro class '"<< className << "' found";
        error(ss.str());
    }
}

void ReaderImpl::onClass(const std::string &className,
        const std::string &subclass)
{
    if (!m_curCell)
    {
        return;
    }

    std::string classNameUpper = toUpper(className);
    std::string subclassUpper = toUpper(subclass);

    if (classNameUpper == "CORE")
    {
        m_curCell->m_subclass = CellSubclass::NONE;
        constexpr const std::array<const char*, 6> optstr
        {{
            "FEEDTHRU","TIEHIGH","TIELOW","SPACER","ANTENNACELL","WELLTAP"
        }};
        
        constexpr const std::array<int, 6> optval
        {{
            CellSubclass::FEEDTHRU,
            CellSubclass::TIEHIGH,
            CellSubclass::TIELOW,
            CellSubclass::SPACER,
            CellSubclass::ANTENNACELL,
            CellSubclass::WELLTAP
        }};

        for(uint32_t i=0; i<optstr.size(); i++)
        {
            if (optstr[i] == subclassUpper)
            {
                m_curCell->m_subclass = optval[i];
                break;
            }
        }        
        
        m_curCell->m_class = CellClass::CORE;
    }
    else if (classNameUpper == "COVER")
    {
        m_curCell->m_class = CellClass::COVER;
        if (subclassUpper != "BUMP")
        {
            std::stringstream ss;
            ss << "Unknown COVER subclass '"<< subclass << "' found";
            error(ss.str());
        }
        else
        {
            m_curCell->m_subclass = CellSubclass::BUMP;
        }
    }
    else if (classNameUpper == "PAD")
    {
        m_curCell->m_subclass = CellSubclass::NONE;
        constexpr const std::array<const char*, 6> optstr =
        {{
            "INPUT","OUTPUT","INOUT","POWER","SPACER","AREAIO"
        }};
        
        constexpr const std::array<int, 6> optval
        {
            CellSubclass::INPUT,
            CellSubclass::OUTPUT,
            CellSubclass::INOUT,
            CellSubclass::POWER,
            CellSubclass::SPACER,
            CellSubclass::AREAIO
        };

        for(uint32_t i=0; i<optstr.size(); i++)
        {
            if (optstr[i] == subclassUpper)
            {
                m_curCell->m_subclass = optval[i];
                break;
            }
        }

        m_curCell->m_class = CellClass::PAD;
    }
    else if (classNameUpper == "ENDCAP")
    {
        m_curCell->m_subclass = CellSubclass::NONE;
        constexpr const std::array<const char*, 6> optstr =
        {{
            "PRE","POST","TOPLEFT","TOPRIGHT","BOTTOMLEFT","BOTTOMRIGHT"
        }};
        
        constexpr const std::array<int, 6> optval
        {
            CellSubclass::PRE,
            CellSubclass::POST,
            CellSubclass::TOPLEFT,
            CellSubclass::TOPRIGHT,
            CellSubclass::BOTTOMLEFT,
            CellSubclass::BOTTOMRIGHT
        };

        for(uint32_t i=0; i<optstr.size(); i++)
        {
            if (optstr[i] == subclassUpper)
            {
                m_curCell->m_subclass = optval[i];
                break;
            }
        }          
        m_curCell->m_class = CellClass::ENDCAP;
    }
    else if (classNameUpper == "RING")
    {
        // RING does not have any subclasses!
        error("RING cannot have a sub class!");
        m_curCell->m_subclass = CellSubclass::NONE;
        m_curCell->m_class = CellClass::RING;
    }
    else if (classNameUpper == "BLOCK")
    {
        m_curCell->m_subclass = CellSubclass::NONE;
        if (subclassUpper == "BLACKBOX")
            m_curCell->m_subclass = CellSubclass::BLACKBOX;
        else if (subclassUpper == "SOFT")
            m_curCell->m_subclass = CellSubclass::SOFT;
        else
        {
            std::stringstream ss;
            ss << "Unknown sub class " << subclass << " for BLOCK";
            error(ss.str());
        }
        m_curCell->m_class = CellClass::BLOCK;
    }    
    else
    {
        std::stringstream ss;
        ss << "Unknown macro class '"<< className << "' found";
        error(ss.str());
    }

}

void ReaderImpl::onSymmetry(const SymmetryFlags &symmetry)
{
    if (!m_curCell)
    {
        return;
    }

    m_curCell->m_symmetry = symmetry;
}

void ReaderImpl::onRect(int64_t x1, int64_t y1, int64_t x2, int64_t y2)
{        
    Rectangle rect({Coord64{std::min(x1,x2), std::min(y1,y2)}, 
        Coord64{std::max(x1,x2), std::max(y1,y2)}});

    switch(m_context)
    {
    case CONTEXT_PIN:
        {             
            if (!m_curPinInfo)
            {
                Logging::doLog(Logging::LogType::ERROR,"LEF::ReaderImpl::onRect m_curPinInfo is null\n");
                return;
            }

            m_curPinInfo->m_pinLayout[m_activePinLayerName].push_back(rect);
        }
        break;
    case CONTEXT_OBS:
        {
            if (!m_curCell)
            {
                return;
            }            
            m_curCell->m_obstructions[m_activeObsLayerName].push_back(rect);
        }        
        break;
    }
}

void ReaderImpl::onPolygon(const std::vector<Coord64> &points)
{
    if (!m_curCell)
    {
        return;
    }

    Polygon poly(points);
    switch(m_context)
    {
    case CONTEXT_PIN:
        if (!m_curPinInfo)
        {
            return;
        }    
        m_curPinInfo->m_pinLayout[m_activePinLayerName].push_back(poly);
        break;
    case CONTEXT_OBS:
        if (!m_curCell)
        {
            return;
        }      
        m_curCell->m_obstructions[m_activeObsLayerName].push_back(poly);
        break;
    }
}

void ReaderImpl::onPortLayer(const std::string &layerName)
{
    m_activePinLayerName = layerName;
}

void ReaderImpl::onObstruction()
{
    m_context = CONTEXT_OBS;
}

void ReaderImpl::onObstructionLayer(const std::string &layerName)
{
    m_activeObsLayerName = layerName;
}

void ReaderImpl::onLayer(const std::string &layerName)
{
    Logging::doLog(Logging::LogType::VERBOSE,"LEF LAYER: %s\n", layerName.c_str());

    auto layerKeyObjPair = m_design.m_techLib->createLayer(layerName);

    m_curLayerInfo = layerKeyObjPair.ptr();
}


void ReaderImpl::onEndLayer(const std::string &layerName)
{
    // Note: if a layer offset is not specified by the LEF
    //       the default is half the pitch!
    // 
    // LEF/DEF 5.7 Language Reference page 103.
    // https://www.ispd.cc/contests/18/lefdefref.pdf

    if (m_curLayerInfo)
    {
        if (m_curLayerInfo->m_offset.m_x < 0)
        {
            m_curLayerInfo->m_offset = ChipDB::Coord64{m_curLayerInfo->m_pitch.m_x/2, m_curLayerInfo->m_pitch.m_y/2};
        }
    }

    m_curLayerInfo.reset();
}

/** callback for layer type */
void ReaderImpl::onLayerType(const std::string &layerType)
{
    if (!m_curLayerInfo)
    {
        Logging::doLog(Logging::LogType::ERROR,"Layer is nullptr\n");
        return;
    }

    using OptionPair = std::pair<const char *, LayerType>;

    constexpr const std::array<OptionPair, 4> validOptions = {{ 
        {"ROUTING", LayerType::ROUTING},
        {"CUT", LayerType::CUT},
        {"MASTERSLICE", LayerType::MASTERSLICE},
        {"OVERLAP", LayerType::OVERLAP}
    }};
    
    std::string layerTypeUpper = toUpper(layerType);

    for(auto option : validOptions)
    {
        if (layerTypeUpper == option.first)
        {
            m_curLayerInfo->m_type = option.second;
            return;
        }
    }

    m_curLayerInfo->m_type = LayerType::UNDEFINED;

    Logging::doLog(Logging::LogType::WARNING, "Unknown layer type in LEF file: %s\n", layerType.c_str());
}

void ReaderImpl::onPinUse(const std::string &use)
{
    m_pinUse = toUpper(use);
}

void ReaderImpl::onPinDirection(const std::string &direction)
{
    m_pinDirection = toUpper(direction);
}

void ReaderImpl::onLayerPitch(int64_t pitchx, int64_t pitchy)
{
    if (!m_curLayerInfo)
    {
        Logging::doLog(Logging::LogType::ERROR,"Layer is nullptr\n");
        return;
    }

    m_curLayerInfo->m_pitch = {pitchx, pitchy};
}

void ReaderImpl::onLayerSpacing(int64_t spacing)
{
    if (!m_curLayerInfo)
    {
        Logging::doLog(Logging::LogType::ERROR,"Layer is nullptr\n");
        return;
    }

    m_curLayerInfo->m_spacing = static_cast<int32_t>(spacing);

}

void ReaderImpl::onLayerSpacingRange(int64_t value1, int64_t value2)
{
    if (!m_curLayerInfo)
    {
        Logging::doLog(Logging::LogType::ERROR,"Layer is nullptr\n");
        return;
    }

    //FIXME: implement this.
}

void ReaderImpl::onLayerOffset(int64_t offsetx, int64_t offsety)
{
    if (!m_curLayerInfo)
    {
        Logging::doLog(Logging::LogType::ERROR,"Layer is nullptr\n");
        return;
    }

    m_curLayerInfo->m_offset = {offsetx, offsety};
}

void ReaderImpl::onLayerDirection(const std::string &direction)
{
    if (!m_curLayerInfo)
    {
        Logging::doLog(Logging::LogType::ERROR,"Layer is nullptr\n");
        return;
    }

    if (direction == "HORIZONTAL")
    {
        m_curLayerInfo->m_dir = LayerDirection::HORIZONTAL;
    }
    else if (direction == "VERTICAL")
    {
        m_curLayerInfo->m_dir = LayerDirection::VERTICAL;
    }
    else
    {
        Logging::doLog(Logging::LogType::WARNING,"Layer direction undefined - got %s\n", direction.c_str());
        m_curLayerInfo->m_dir = LayerDirection::UNDEFINED;
    }
    
    // Other routing directions are not supported.
}

void ReaderImpl::onLayerWidth(int64_t width)
{
    if (!m_curLayerInfo)
    {
        Logging::doLog(Logging::LogType::ERROR,"Layer is nullptr\n");
        return;
    }

    m_curLayerInfo->m_width = static_cast<int32_t>(width);
}

void ReaderImpl::onLayerMaxWidth(int64_t maxWidth)
{
    if (!m_curLayerInfo)
    {
        Logging::doLog(Logging::LogType::ERROR,"Layer is nullptr\n");
        return;
    }

    m_curLayerInfo->m_maxWidth = static_cast<int32_t>(maxWidth);
}

void ReaderImpl::onManufacturingGrid(int64_t grid)
{
    m_design.m_techLib->m_manufacturingGrid = grid;
}


void ReaderImpl::onLayerResistancePerSq(double ohms)
{
    if (!m_curLayerInfo)
    {
        Logging::doLog(Logging::LogType::ERROR,"Layer is nullptr\n");
        return;
    }

    m_curLayerInfo->m_resistance = ohms;
}

void ReaderImpl::onLayerCapacitancePerSq(double farads)
{
    if (!m_curLayerInfo)
    {
        Logging::doLog(Logging::LogType::ERROR,"Layer is nullptr\n");
        return;
    }

    m_curLayerInfo->m_capacitance = farads;
}

void ReaderImpl::onLayerEdgeCapacitance(double farads)
{
    if (!m_curLayerInfo)
    {
        Logging::doLog(Logging::LogType::ERROR,"Layer is nullptr\n");
        return;
    }

    m_curLayerInfo->m_edgeCapacitance = farads;
}

void ReaderImpl::onLayerThickness(double thickness)
{
    if (!m_curLayerInfo)
    {
        Logging::doLog(Logging::LogType::ERROR,"Layer is nullptr\n");
        return;
    }

    m_curLayerInfo->m_thickness = thickness;
}

void ReaderImpl::onLayerMinArea(double minArea)
{
    if (m_curLayerInfo == nullptr)
    {
        Logging::doLog(Logging::LogType::ERROR,"Layer is nullptr\n");
        return;
    }

    m_curLayerInfo->m_minArea = minArea;
}

void ReaderImpl::onSite(const std::string &site)
{
    auto siteKeyObjPair = m_design.m_techLib->createSiteInfo(site);
    m_curSiteInfo = siteKeyObjPair.ptr();
    Logging::doLog(Logging::LogType::VERBOSE,"LEF SITE: %s\n", site.c_str());
}

void ReaderImpl::onEndSite(const std::string &site)
{
    if (!m_curSiteInfo)
    {
        Logging::doLog(Logging::LogType::ERROR,"Site is nullptr\n");
        return;
    }

    m_curSiteInfo.reset();
}

void ReaderImpl::onSiteSize(int64_t x, int64_t y)
{
    if (!m_curSiteInfo)
    {
        Logging::doLog(Logging::LogType::ERROR,"Site is nullptr\n");
        return;
    }

    m_curSiteInfo->m_size = Coord64{x,y};
}


void ReaderImpl::onSiteSymmetry(const SymmetryFlags &symmetry)
{
    if (!m_curSiteInfo)
    {
        Logging::doLog(Logging::LogType::ERROR,"Site is nullptr\n");
        return;
    }

    m_curSiteInfo->m_symmetry = symmetry;
}


void ReaderImpl::onSiteClass(const std::string &siteClass)
{
    if (!m_curSiteInfo)
    {
        Logging::doLog(Logging::LogType::ERROR,"Site is nullptr\n");
        return;
    }

    if (siteClass == "CORE")
    {
        m_curSiteInfo->m_class = SiteClass::CORE;
    }
    else if (siteClass == "PAD")
    {
        m_curSiteInfo->m_class = SiteClass::PAD;
    }
    else
    {
        std::stringstream ss;
        ss << "Unrecognised site class '" << siteClass << "'in LEF\n";
        Logging::doLog(Logging::LogType::WARNING, ss);
    }
}
