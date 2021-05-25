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

    m_curCell = nullptr;
    m_curPinInfo = nullptr;
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
    if (!checkPtr(m_curCell))
        return;

    // sx, sy are in nanometers
    m_curCell->m_size.m_x = sx;
    m_curCell->m_size.m_y = sy;
    m_curCell->m_area = (sx/1000.0) * (sy/1000.0);
}

void ReaderImpl::onMacroSite(const std::string &site)
{
    if (!checkPtr(m_curCell))
        return;

    m_curCell->m_site = site;
}

void ReaderImpl::onPin(const std::string &pinName)
{
    if (!checkPtr(m_curCell))
        return;

    // find already existing pin or create one.
    m_curPinInfo = &m_curCell->createPin(pinName);
    m_context = CONTEXT_PIN;
}

void ReaderImpl::onEndPin(const std::string &pinName)
{
    if (!checkPtr(m_curCell))
        return;

    if (!checkPtr(m_curPinInfo))
        return;

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
        return;
#if 0
    if (className == "CORE")
    {
        cell->m_class = "C_CORE";
    }
    else if (className == "COVER")
    {
        cell->m_class = "C_COVER";
    }
    else if (className == "PAD")
    {
        cell->m_class = "C_PAD";
    }
    else if (className == "ENDCAP")
    {
        cell->m_class = "C_ENDCAP";
    }
    else if (className == "RING")
    {
        cell->m_class = "C_RING";
    }
    else
    {
        std::stringstream ss;
        ss << "Unknown macro class '"<< className << "' found";
        error(ss.str());
    }
#endif
}

void ReaderImpl::onClass(const std::string &className,
        const std::string &subclass)
{
    if (!checkPtr(m_curCell))
        return;

#if 0
    if (className == "CORE")
    {
        cell->m_subclass = "ST_NONE";
        const std::array<std::string, 6> optstr =
        {
            "FEEDTHRU","TIEHIGH","TIELOW","SPACER","ANTENNACELL","WELLTAP"
        };
        
        const std::array<std::string, 6> optval
        {
            "ST_FEEDTHRU",
            "ST_TIEHIGH",
            "ST_TIELOW",
            "ST_SPACER",
            "ST_ANTENNACELL",
            "ST_WELLTAP"
        };

        for(uint32_t i=0; i<optstr.size(); i++)
        {
            if (optstr[i] == subclass)
            {
                cell->m_subclass = optval[i];
                break;
            }
        }        
        
        cell->m_class = "C_CORE";
    }
    else if (className == "COVER")
    {
        cell->m_class = "C_COVER";
        if (subclass != "BUMP")
        {
            std::stringstream ss;
            ss << "Unknown COVER subclass '"<< subclass << "' found";
            error(ss.str());
        }
        else
        {
            cell->m_subclass = "ST_BUMP";
        }
    }
    else if (className == "PAD")
    {
        cell->m_subclass = "ST_NONE";
        const std::array<std::string, 6> optstr =
        {
            "INPUT","OUTPUT","INOUT","POWER","SPACER","AREAIO"
        };
        
        const std::array<std::string, 6> optval
        {
            "ST_INPUT",
            "ST_OUTPUT",
            "ST_INOUT",
            "ST_POWER",
            "ST_SPACER",
            "ST_AREAIO"
        };

        for(uint32_t i=0; i<optstr.size(); i++)
        {
            if (optstr[i] == subclass)
            {
                cell->m_subclass = optval[i];
                break;
            }
        }

        cell->m_class = "C_PAD";
    }
    else if (className == "ENDCAP")
    {
        cell->m_subclass = "ST_NONE";
        const std::array<std::string, 6> optstr =
        {
            "PRE","POST","TOPLEFT","TOPRIGHT","BOTTOMLEFT","BOTTOMRIGHT"
        };
        
        const std::array<std::string, 6> optval
        {
            "ST_PRE",
            "ST_POST",
            "ST_TOPLEFT",
            "ST_TOPRIGHT",
            "ST_BOTTOMLEFT",
            "ST_BOTTOMRIGHT"
        };

        for(uint32_t i=0; i<optstr.size(); i++)
        {
            if (optstr[i] == subclass)
            {
                cell->m_subclass = optval[i];
                break;
            }
        }          
        cell->m_class = "C_ENDCAP";
    }
    else if (className == "RING")
    {
        // RING does not have any subclasses!
        error("RING cannot have a sub class!");
        cell->m_subclass = "ST_NONE";
        cell->m_class = "C_RING";
    }
    else if (className == "BLOCK")
    {
        cell->m_subclass = "ST_NONE";
        if (subclass == "BLACKBOX")
            cell->m_subclass = "ST_BLACKBOX";
        else if (subclass == "SOFT")
            cell->m_subclass = "ST_SOFT";
        else
        {
            std::stringstream ss;
            ss << "Unknown sub class " << subclass << " for BLOCK";
            error(ss.str());
        }
        cell->m_class = "C_BLOCK";
    }    
    else
    {
        std::stringstream ss;
        ss << "Unknown macro class '"<< className << "' found";
        error(ss.str());
    }
#endif
}

void ReaderImpl::onSymmetry(const SymmetryFlags &symmetry)
{
    if (!checkPtr(m_curCell))
        return;

    m_curCell->m_symmetry = symmetry;
}

void ReaderImpl::onRect(int64_t x1, int64_t y1, int64_t x2, int64_t y2)
{
    if (!checkPtr(m_curCell))
        return;

    if (!checkPtr(m_curPinInfo))
        return;

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
        return;

    if (!checkPtr(m_curPinInfo))
        return;

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
    #if 0
    doLog(LOG_VERBOSE,"LEF LAYER: %s\n", layerName.c_str());

    m_layerIndex = m_design->createLayerInfo(layerName);

    auto layerPtr = m_design->layerInfos().lookup(layerName);
    if (layerPtr == nullptr)
    {
        doLog(LOG_ERROR,"Cannot find layer info %s\n", layerName.c_str());
        return;
    }

    layerPtr->m_dir = "UNDEFINED";

    // set some sensible defaults
    layerPtr->m_maxWidth = 0;
    layerPtr->m_pitch    = {0,0};
    layerPtr->m_spacing  = 0;
    layerPtr->m_width    = 0;
    layerPtr->m_offset   = {0,0};
    layerPtr->m_type = "UNDEFINED";
#endif
}


void ReaderImpl::onEndLayer(const std::string &layerName)
{
    //m_layerIndex = -1;
}

/** callback for layer type */
void ReaderImpl::onLayerType(const std::string &layerType)
{
#if 0
    //FIXME: make NPContainer return a pointer on []
    auto layerPtr = &m_design->layerInfos()[m_layerIndex];
    if (layerPtr == nullptr)
    {
        doLog(LOG_ERROR,"Layer is nullptr\n");
        return;
    }

    const std::vector<std::string> validOptions = {"ROUTING","CUT","MASTERSLICE","OVERLAP"};
    
    auto iter = std::find(validOptions.begin(), validOptions.end(), layerType);

    if (iter != validOptions.end())
    {
        layerPtr->m_type = layerType;
    }
    else
    {
        doLog(LOG_WARN, "Unknown layer type in LEF file: %s\n", layerType.c_str());
    }
#endif
}

void ReaderImpl::onPinUse(const std::string &use)
{
    m_pinUse = use;
}

void ReaderImpl::onPinDirection(const std::string &direction)
{
    m_pinDirection = direction;
}

void ReaderImpl::onLayerPitch(int64_t pitchx, int64_t pitchy)
{
#if 0    
    //FIXME: make NPContainer return a pointer on []
    auto layerPtr = &m_design->layerInfos()[m_layerIndex];
    if (layerPtr == nullptr)
    {
        doLog(LOG_ERROR,"Layer is nullptr\n");
        return;
    }

    layerPtr->m_pitch = {pitchx, pitchy};
#endif
}

void ReaderImpl::onLayerSpacing(int64_t spacing)
{
#if 0
    //FIXME: make NPContainer return a pointer on []
    auto layerPtr = &m_design->layerInfos()[m_layerIndex];
    if (layerPtr == nullptr)
    {
        doLog(LOG_ERROR,"Layer is nullptr\n");
        return;
    }

    layerPtr->m_spacing = static_cast<int32_t>(spacing);
#endif
}

void ReaderImpl::onLayerSpacingRange(int64_t value1, int64_t value2)
{
    #if 0
    //FIXME: make NPContainer return a pointer on []
    auto layerPtr = &m_design->layerInfos()[m_layerIndex];
    if (layerPtr == nullptr)
    {
        doLog(LOG_ERROR,"Layer is nullptr\n");
        return;
    }

    //TODO:
#endif
}

void ReaderImpl::onLayerOffset(int64_t offsetx, int64_t offsety)
{
#if 0
    //FIXME: make NPContainer return a pointer on []
    auto layerPtr = &m_design->layerInfos()[m_layerIndex];
    if (layerPtr == nullptr)
    {
        doLog(LOG_ERROR,"Layer is nullptr\n");
        return;
    }

    layerPtr->m_offset = {offsetx, offsety};
#endif
}

void ReaderImpl::onLayerDirection(const std::string &direction)
{
#if 0
    //FIXME: make NPContainer return a pointer on []
    auto layerPtr = &m_design->layerInfos()[m_layerIndex];
    if (layerPtr == nullptr)
    {
        doLog(LOG_ERROR,"Layer is nullptr\n");
        return;
    }

    if (direction == "HORIZONTAL")
    {
        layerPtr->m_dir = "HORIZONTAL";
    }
    else if (direction == "VERTICAL")
    {
        layerPtr->m_dir = "VERTICAL";
    }
    else
    {
        doLog(LOG_WARN,"Layer direction undefined - got %s\n", direction.c_str());
        layerPtr->m_dir = "UNDEFINED";
    }
    // Other routing directions are not supported.
#endif
}

void ReaderImpl::onLayerWidth(int64_t width)
{
#if 0
    //FIXME: make NPContainer return a pointer on []
    auto layerPtr = &m_design->layerInfos()[m_layerIndex];
    if (layerPtr == nullptr)
    {
        doLog(LOG_ERROR,"Layer is nullptr\n");
        return;
    }

    layerPtr->m_width = static_cast<int32_t>(width);
#endif
}

void ReaderImpl::onLayerMaxWidth(int64_t maxWidth)
{
#if 0
    //FIXME: make NPContainer return a pointer on []
    auto layerPtr = &m_design->layerInfos()[m_layerIndex];
    if (layerPtr == nullptr)
    {
        doLog(LOG_ERROR,"Layer is nullptr\n");
        return;
    }

    layerPtr->m_maxWidth = static_cast<int32_t>(maxWidth);
#endif
}

void ReaderImpl::onManufacturingGrid(int64_t grid)
{
#if 0
    m_design->techLib()->setManufacturingGrid(grid);
#endif
}


void ReaderImpl::onLayerResistancePerSq(double ohms)
{
#if 0
    //FIXME: make NPContainer return a pointer on []
    auto layerPtr = &m_design->layerInfos()[m_layerIndex];
    if (layerPtr == nullptr)
    {
        doLog(LOG_ERROR,"Layer is nullptr\n");
        return;
    }

    layerPtr->m_resistance = ohms;
#endif
}

void ReaderImpl::onLayerCapacitancePerSq(double farads)
{
    #if 0
    //FIXME: make NPContainer return a pointer on []
    auto layerPtr = &m_design->layerInfos()[m_layerIndex];
    if (layerPtr == nullptr)
    {
        doLog(LOG_ERROR,"Layer is nullptr\n");
        return;
    }

    layerPtr->m_capacitance = farads;
#endif    
}

void ReaderImpl::onLayerEdgeCapacitance(double farads)
{
#if 0
    //FIXME: make NPContainer return a pointer on []
    auto layerPtr = &m_design->layerInfos()[m_layerIndex];
    if (layerPtr == nullptr)
    {
        doLog(LOG_ERROR,"Layer is nullptr\n");
        return;
    }

    layerPtr->m_edgeCapacitance = farads;
#endif
}

void ReaderImpl::onLayerThickness(double thickness)
{
#if 0
    //FIXME: make NPContainer return a pointer on []
    auto layerPtr = &m_design->layerInfos()[m_layerIndex];
    if (layerPtr == nullptr)
    {
        doLog(LOG_ERROR,"Layer is nullptr\n");
        return;
    }

    layerPtr->m_thickness = thickness;
#endif
}

void ReaderImpl::onLayerMinArea(double minArea)
{
#if 0
    //FIXME: make NPContainer return a pointer on []
    auto layerPtr = &m_design->layerInfos()[m_layerIndex];
    if (layerPtr == nullptr)
    {
        doLog(LOG_ERROR,"Layer is nullptr\n");
        return;
    }

    layerPtr->m_minArea = minArea;
#endif
}

void ReaderImpl::onSite(const std::string &site)
{
#if 0
    m_siteIndex = m_design->techLib()->createSiteInfo(site);
    doLog(LOG_VERBOSE,"LEF SITE: %s\n", site.c_str());
#endif
}

void ReaderImpl::onEndSite(const std::string &site)
{
#if 0
    //FIXME: make NPContainer return a pointer on []
    auto sitePtr = &m_design->siteInfos()[m_siteIndex];
    if (sitePtr == nullptr)
    {
        doLog(LOG_ERROR,"Site is nullptr\n");
        return;
    }
#endif
}

void ReaderImpl::onSiteSize(int64_t x, int64_t y)
{
#if 0
    //FIXME: make NPContainer return a pointer on []
    auto sitePtr = &m_design->siteInfos()[m_siteIndex];
    if (sitePtr == nullptr)
    {
        doLog(LOG_ERROR,"Site is nullptr\n");
        return;
    }

    Coord64 s{x,y};
    sitePtr->m_size = s;
#endif
}

#if 0
void ReaderImpl::onSiteSymmetry(const SymmetryType &symmetry)
{

    //FIXME: make NPContainer return a pointer on []
    auto sitePtr = &m_design->siteInfos()[m_siteIndex];
    if (sitePtr == nullptr)
    {
        doLog(LOG_ERROR,"Site is nullptr\n");
        return;
    }

    sitePtr->m_symmetry = symmetry;

}
#endif

void ReaderImpl::onSiteClass(const std::string &siteClass)
{
#if 0
    //FIXME: make NPContainer return a pointer on []
    auto sitePtr = &m_design->siteInfos()[m_siteIndex];
    if (sitePtr == nullptr)
    {
        doLog(LOG_ERROR,"Site is nullptr\n");
        return;
    }

    if (siteClass == "CORE")
    {
        sitePtr->m_class = "CORE";
    }
    else if (siteClass == "PAD")
    {
        sitePtr->m_class = "PAD";
    }
#endif
}

