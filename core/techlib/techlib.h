#pragma once

#include "common/dbtypes.h"
#include "common/visitor.h"
#include "common/namedstorage.h"

namespace ChipDB
{

enum LayerType : uint8_t
{
    LAYER_UNDEFINED = 0,
    LAYER_ROUTING,
    LAYER_CUT,
    LAYER_MASTERSLICE,
    LAYER_OVERLAP
};

std::string toString(const LayerType &lt);

enum LayerDirection : uint8_t
{
    LAYERDIR_UNDEFINED = 0,
    LAYERDIR_HORIZONTAL,
    LAYERDIR_VERTICAL
};

std::string toString(const LayerDirection &lt);


struct LayerInfo
{
    LayerInfo() 
    :   m_pitch{0,0},
        m_spacing(0),
        m_width(0),
        m_maxWidth(0),
        m_offset{0,0},
        m_edgeCapacitance(0.0),
        m_capacitance(0.0),
        m_resistance(0.0),
        m_thickness(0.0),
        m_minArea(0.0),
        m_type(LAYER_UNDEFINED),
        m_dir(LAYERDIR_UNDEFINED)
    {}

    LayerInfo(const std::string &name) 
    : m_name(name),
        m_pitch{0,0},
        m_spacing(0),
        m_width(0),
        m_maxWidth(0),
        m_offset{0,0},
        m_edgeCapacitance(0.0),
        m_capacitance(0.0),
        m_resistance(0.0),
        m_thickness(0.0),
        m_minArea(0.0),
        m_type(LAYER_UNDEFINED),
        m_dir(LAYERDIR_UNDEFINED)        
    {}

    IMPLEMENT_ACCEPT;

    std::string m_name;

    Coord64 m_pitch;    ///< in nm in x and y direction.
    int32_t m_spacing;  ///< in nm
    int32_t m_width;    ///< in nm
    int32_t m_maxWidth; ///< in nm
    Coord64 m_offset;   ///< in nm in x and y direction.

    double  m_edgeCapacitance;  ///< in farads per micron.
    double  m_capacitance;      ///< in farads per square micron.
    double  m_resistance;       ///< in ohms per square, or restance per cut (cut layer only)

    double  m_thickness;
    double  m_minArea;

    LayerType       m_type;
    LayerDirection  m_dir;
};

#if 0

/* StrEnum for layer type */
struct SiteClass : public StrEnum<SiteClass>
{
    SiteClass() : StrEnum() {}
    SiteClass(const std::string &s) { *this = s; }
    
    static constexpr std::array<std::pair<int32_t, const char *>, 3> m_options =
    {{
        {0,"UNDEFINED"},
        {1,"PAD"},
        {2,"CORE"}
    }};

    StrEnumOverloads(SiteClass);
};

struct SiteInfo : public NamedObject
{
    SiteInfo()
    : NamedObject(""),
      m_size{0,0},
      m_class("UNDEFINED") {}
    
    SiteInfo(const std::string &name) 
    : NamedObject(name),
      m_size{0,0},
      m_class("UNDEFINED") {}

    IMPLEMENT_ACCEPT;

    Coord64         m_size;
    SiteClass       m_class;
    SymmetryType    m_symmetry;
};

#endif

class TechLib
{
public:
    TechLib() : 
        m_manufacturingGrid(0)
        {}

    NamedStorage<LayerInfo*> m_layers;
    int32_t                  m_manufacturingGrid; // in nm.

    size_t getNumberOfLayers() const
    {
        return m_layers.size();
    };

    auto begin() const
    {
        return m_layers.begin();
    }

    auto end() const
    {
        return m_layers.end();
    }

    LayerInfo* createLayer(const std::string &name);
    LayerInfo* lookup(const std::string &name) const;

#if 0
    SiteInfoIndex createSiteInfo(const std::string &name);

    constexpr auto& siteInfos()
    {
        return m_siteInfos;
    }

    constexpr auto const& siteInfos() const
    {
        return m_siteInfos;
    }
#endif   
};

};
