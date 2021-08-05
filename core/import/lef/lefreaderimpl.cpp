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

#include <vector>
#include <array>
#include <string>
#include "lefreaderimpl.h"
#include "netlist/design.h"
#include "common/logging.h"

using namespace ChipDB::LEF;

ReaderImpl::ReaderImpl(Design *design) 
    : m_design(design),    
      m_activeObsLayerIdx(0),
      m_activePinLayerIdx(0)  
{
    m_context = CONTEXT_PIN;

    m_curCell       = nullptr;
    m_curPinInfo    = nullptr;
    m_curLayerInfo  = nullptr;
    m_curSiteInfo   = nullptr;
}

void ReaderImpl::onMacro(const std::string &macroName)
{
    // create a new cell/macro if necessary    
    m_curCell = m_design->m_cellLib.createCell(macroName);

    doLog(LOG_VERBOSE,"LEF MACRO: %s\n", macroName.c_str());
}

void ReaderImpl::onEndMacro(const std::string &macroName)
{      
    m_curCell = nullptr;
}

void ReaderImpl::onSize(int64_t sx, int64_t sy)
{   
    const double nm2microns = 0.001;

    if (!checkPtr(m_curCell))
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
    if (!checkPtr(m_curCell))
    {
        return;
    }

    m_curCell->m_site = site;
}

void ReaderImpl::onPin(const std::string &pinName)
{
    if (!checkPtr(m_curCell))
    {
        return;
    }

    // find already existing pin or create one.
    m_curPinInfo = &m_curCell->createPin(pinName);
    m_context = CONTEXT_PIN;
}

void ReaderImpl::onEndPin(const std::string &pinName)
{
    if (!checkPtr(m_curCell))
    {
        return;
    }

    if (!checkPtr(m_curPinInfo))
    {
        return;
    }

    // figure out the type of pin
    if (m_pinUse == "GROUND")
    {
        m_curPinInfo->m_iotype = IO_GROUND;
    }
    else if (m_pinUse == "POWER")
    {
        m_curPinInfo->m_iotype = IO_POWER;
    }
    else if (m_pinDirection == "INOUT")
    {
        m_curPinInfo->m_iotype = IO_IO;
    }
    else if (m_pinDirection == "INPUT")
    {
        // Note: INPUT pins may later be promoted to 
        //       CLOCK type when the Liberty file has been
        //       read
        m_curPinInfo->m_iotype = IO_INPUT;
    }
    else if (m_pinDirection == "OUTPUT")
    {
        m_curPinInfo->m_iotype = IO_OUTPUT;
    }
    else if (m_pinDirection == "OUTPUT TRISTATE")
    {
        m_curPinInfo->m_iotype = IO_OUTPUT_TRI;
    }    
    else
    {
        m_curPinInfo->m_iotype = IO_UNKNOWN;
    }

    m_pinDirection.clear();
    m_pinUse.clear();
}

void ReaderImpl::onClass(const std::string &className)
{
    if (!checkPtr(m_curCell))
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
    if (!checkPtr(m_curCell))
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
        
        constexpr const std::array<CellSubclass, 6> optval
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
        
        constexpr const std::array<CellSubclass, 6> optval
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
        
        constexpr const std::array<CellSubclass, 6> optval
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
    if (!checkPtr(m_curCell))
    {
        return;
    }

    m_curCell->m_symmetry = symmetry;
}

void ReaderImpl::onRect(int64_t x1, int64_t y1, int64_t x2, int64_t y2)
{
    if (!checkPtr(m_curCell))
    {
        return;
    }

    if (!checkPtr(m_curPinInfo))
    {
        return;
    }

#if 0
    CellRect *rect = new CellRect();

    rect->m_rect.setLL(Coord64{std::min(x1,x2), std::min(y1,y2)});
    rect->m_rect.setUR(Coord64{std::max(x1,x2), std::max(y1,y2)});

    switch(m_context)
    {
    case CONTEXT_PIN:
        { 
            auto pinLayout = cell->pinLayout(m_pinIndex);
            if (pinLayout != nullptr)
            {
                rect->m_layerIndex = m_activePinLayerIdx;
                pinLayout->push_back(rect);
            }
            else
            {
                doLog(LOG_ERROR,"LEF::Reader pin layout not found for pin %s\n", pin->getName());
            }
        }
        break;
    case CONTEXT_OBS:
        rect->m_layerIndex = m_activeObsLayerIdx;
        cell->obstructions().push_back(rect);
        break;
    default:
        delete rect;
        break;
    }
#endif
}

void ReaderImpl::onPolygon(const std::vector<Coord64> &points)
{
    if (!checkPtr(m_curCell))
    {
        return;
    }

    if (!checkPtr(m_curPinInfo))
    {
        return;
    }

#if 0
    auto pin = &cell->pins()[m_pinIndex];

    CellPolygon *poly = new CellPolygon();
    poly->m_poly.m_points = points;

    switch(m_context)
    {
    case CONTEXT_PIN:
        {
            auto pinLayout = cell->pinLayout(m_pinIndex);
            if (pinLayout != nullptr)
            {
                poly->m_layerIndex = m_activePinLayerIdx;
                pinLayout->push_back(poly);
            }
            else
            {
                doLog(LOG_ERROR,"LEF::Reader pin layout not found for pin %s\n", pin->getName());
            }
        }    
        break;
    case CONTEXT_OBS:
        poly->m_layerIndex = m_activeObsLayerIdx;
        cell->obstructions().push_back(poly);
        break;
    default:
        delete poly;
    }
#endif

}

void ReaderImpl::onPortLayer(const std::string &layerName)
{
    //FIXME:
#if 0
    auto layerInfoIndex = m_design->techLib()->layerInfos().indexOf(layerName);
    if (layerInfoIndex >= 0)
    {
        m_activePinLayerIdx = layerInfoIndex;
    }
    else
    {        
        doLog(LOG_WARN,"CellLEFReader: cannot find PORT layer %s in database\n", layerName.c_str());
        m_activePinLayerIdx = 0;
    }
#endif
}

void ReaderImpl::onObstruction()
{
    m_context = CONTEXT_OBS;
}

void ReaderImpl::onObstructionLayer(const std::string &layerName)
{
    //FIXME:
    #if 0
    auto layerInfoIndex = m_design->techLib()->layerInfos().indexOf(layerName);
    if (layerInfoIndex >= 0)
    {
        m_activeObsLayerIdx = layerInfoIndex;
    }
    else
    {
        doLog(LOG_WARN,"CellLEFReader: cannot find OBS layer %s in database\n", layerName.c_str());
        m_activeObsLayerIdx = 0;
    }
#endif
}

void ReaderImpl::onLayer(const std::string &layerName)
{
    doLog(LOG_VERBOSE,"LEF LAYER: %s\n", layerName.c_str());

    m_curLayerInfo = m_design->m_techLib.createLayer(layerName);

}


void ReaderImpl::onEndLayer(const std::string &layerName)
{
    m_curLayerInfo = nullptr;
}

/** callback for layer type */
void ReaderImpl::onLayerType(const std::string &layerType)
{
    if (m_curLayerInfo == nullptr)
    {
        doLog(LOG_ERROR,"Layer is nullptr\n");
        return;
    }

    using OptionPair = std::pair<const char *, LayerType>;

    constexpr const std::array<OptionPair, 4> validOptions = {{ 
        {"ROUTING", LAYER_ROUTING},
        {"CUT", LAYER_CUT},
        {"MASTERSLICE", LAYER_MASTERSLICE},
        {"OVERLAP", LAYER_OVERLAP}
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

    m_curLayerInfo->m_type = LAYER_UNDEFINED;

    doLog(LOG_WARN, "Unknown layer type in LEF file: %s\n", layerType.c_str());
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
    if (m_curLayerInfo == nullptr)
    {
        doLog(LOG_ERROR,"Layer is nullptr\n");
        return;
    }

    m_curLayerInfo->m_pitch = {pitchx, pitchy};
}

void ReaderImpl::onLayerSpacing(int64_t spacing)
{
    if (m_curLayerInfo == nullptr)
    {
        doLog(LOG_ERROR,"Layer is nullptr\n");
        return;
    }

    m_curLayerInfo->m_spacing = static_cast<int32_t>(spacing);

}

void ReaderImpl::onLayerSpacingRange(int64_t value1, int64_t value2)
{
    if (m_curLayerInfo == nullptr)
    {
        doLog(LOG_ERROR,"Layer is nullptr\n");
        return;
    }

    //FIXME: implement this.
}

void ReaderImpl::onLayerOffset(int64_t offsetx, int64_t offsety)
{
    if (m_curLayerInfo == nullptr)
    {
        doLog(LOG_ERROR,"Layer is nullptr\n");
        return;
    }

    m_curLayerInfo->m_offset = {offsetx, offsety};
}

void ReaderImpl::onLayerDirection(const std::string &direction)
{
    if (m_curLayerInfo == nullptr)
    {
        doLog(LOG_ERROR,"Layer is nullptr\n");
        return;
    }

    if (direction == "HORIZONTAL")
    {
        m_curLayerInfo->m_dir = LAYERDIR_HORIZONTAL;
    }
    else if (direction == "VERTICAL")
    {
        m_curLayerInfo->m_dir = LAYERDIR_VERTICAL;
    }
    else
    {
        doLog(LOG_WARN,"Layer direction undefined - got %s\n", direction.c_str());
        m_curLayerInfo->m_dir = LAYERDIR_UNDEFINED;
    }
    
    // Other routing directions are not supported.
}

void ReaderImpl::onLayerWidth(int64_t width)
{
    if (m_curLayerInfo == nullptr)
    {
        doLog(LOG_ERROR,"Layer is nullptr\n");
        return;
    }

    m_curLayerInfo->m_width = static_cast<int32_t>(width);
}

void ReaderImpl::onLayerMaxWidth(int64_t maxWidth)
{
    if (m_curLayerInfo == nullptr)
    {
        doLog(LOG_ERROR,"Layer is nullptr\n");
        return;
    }

    m_curLayerInfo->m_maxWidth = static_cast<int32_t>(maxWidth);
}

void ReaderImpl::onManufacturingGrid(int64_t grid)
{
    if (m_design == nullptr)
    {
        doLog(LOG_ERROR,"Design is nullptr\n");
        return;
    }

    m_design->m_techLib.m_manufacturingGrid = grid;
}


void ReaderImpl::onLayerResistancePerSq(double ohms)
{
    if (m_curLayerInfo == nullptr)
    {
        doLog(LOG_ERROR,"Layer is nullptr\n");
        return;
    }

    m_curLayerInfo->m_resistance = ohms;
}

void ReaderImpl::onLayerCapacitancePerSq(double farads)
{
    if (m_curLayerInfo == nullptr)
    {
        doLog(LOG_ERROR,"Layer is nullptr\n");
        return;
    }

    m_curLayerInfo->m_capacitance = farads;
}

void ReaderImpl::onLayerEdgeCapacitance(double farads)
{
    if (m_curLayerInfo == nullptr)
    {
        doLog(LOG_ERROR,"Layer is nullptr\n");
        return;
    }

    m_curLayerInfo->m_edgeCapacitance = farads;

}

void ReaderImpl::onLayerThickness(double thickness)
{
    if (m_curLayerInfo == nullptr)
    {
        doLog(LOG_ERROR,"Layer is nullptr\n");
        return;
    }

    m_curLayerInfo->m_thickness = thickness;
}

void ReaderImpl::onLayerMinArea(double minArea)
{
    if (m_curLayerInfo == nullptr)
    {
        doLog(LOG_ERROR,"Layer is nullptr\n");
        return;
    }

    m_curLayerInfo->m_minArea = minArea;
}

void ReaderImpl::onSite(const std::string &site)
{
    m_curSiteInfo = m_design->m_techLib.createSiteInfo(site);
    doLog(LOG_VERBOSE,"LEF SITE: %s\n", site.c_str());
}

void ReaderImpl::onEndSite(const std::string &site)
{
    if (m_curSiteInfo == nullptr)
    {
        doLog(LOG_ERROR,"Site is nullptr\n");
        return;
    }

    m_curSiteInfo = nullptr;
}

void ReaderImpl::onSiteSize(int64_t x, int64_t y)
{
    if (m_curSiteInfo == nullptr)
    {
        doLog(LOG_ERROR,"Site is nullptr\n");
        return;
    }

    m_curSiteInfo->m_size = Coord64{x,y};
}


void ReaderImpl::onSiteSymmetry(const SymmetryFlags &symmetry)
{
    if (m_curSiteInfo == nullptr)
    {
        doLog(LOG_ERROR,"Site is nullptr\n");
        return;
    }

    m_curSiteInfo->m_symmetry = symmetry;
}


void ReaderImpl::onSiteClass(const std::string &siteClass)
{
    if (m_curSiteInfo == nullptr)
    {
        doLog(LOG_ERROR,"Site is nullptr\n");
        return;
    }

    if (siteClass == "CORE")
    {
        m_curSiteInfo->m_class = SC_CORE;
    }
    else if (siteClass == "PAD")
    {
        m_curSiteInfo->m_class = SC_PAD;
    }
    else
    {
        std::stringstream ss;
        ss << "Unrecognised site class '" << siteClass << "'in LEF\n";
        doLog(LOG_WARN, ss);
    }
}
