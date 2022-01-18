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

    LayerType       m_type;     ///< layer type according to LEF/DEF
    LayerDirection  m_dir;      ///< preferred routing direction
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

};

#endif

enum SiteClass : uint8_t
{
    SC_UNDEFINED = 0,
    SC_PAD,
    SC_CORE
};

struct SiteInfo
{
    SiteInfo()
    : m_size{0,0},
      m_class(SC_UNDEFINED) {}
    
    SiteInfo(const std::string &name) 
    : m_name(name),
      m_size{0,0},
      m_class(SC_UNDEFINED) {}

    //IMPLEMENT_ACCEPT;

    std::string     m_name;         ///< name of the site
    Coord64         m_size;         ///< cell size of the site
    SiteClass       m_class;        ///< class type
    SymmetryFlags   m_symmetry;     ///< supported symmetries
};


class TechLib
{
public:
    TechLib() : m_manufacturingGrid(0) {}

    NamedStorage<LayerInfo>  m_layers;
    NamedStorage<SiteInfo>   m_sites;
    int32_t                  m_manufacturingGrid; // in nm.

    void clear();

    size_t getNumberOfLayers() const
    {
        return m_layers.size();
    };

    size_t getNumberOfSites() const
    {
        return m_sites.size();
    };

    LayerInfo* createLayer(const std::string &name);
    LayerInfo* lookupLayer(const std::string &name) const;

    SiteInfo* createSiteInfo(const std::string &name);
    SiteInfo* lookupSiteInfo(const std::string &name) const; 
};

};
