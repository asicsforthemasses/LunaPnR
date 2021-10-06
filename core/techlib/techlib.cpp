#include "techlib.h"
#include <algorithm>

using namespace ChipDB;

std::string ChipDB::toString(const LayerType &lt)
{
    switch(lt)    
    {
    default:
    case LAYER_UNDEFINED:
        return std::string("UNDEFINED");
    case LAYER_ROUTING:
        return std::string("ROUTING");        
    case LAYER_CUT:
        return std::string("CUT");
    case LAYER_MASTERSLICE:
        return std::string("MASTERSLICE");
    case LAYER_OVERLAP:
        return std::string("OVERLAP");
    }    
}

std::string ChipDB::toString(const LayerDirection &ldir)
{
    switch(ldir)    
    {
    default:
    case LAYERDIR_UNDEFINED:
        return std::string("UNDEFINED");
    case LAYERDIR_HORIZONTAL:
        return std::string("HORIZONTAL");
    case LAYERDIR_VERTICAL:
        return std::string("VERTICAL");
    }
}

void TechLib::clear()
{
    m_layers.clear();
    m_sites.clear();
    m_manufacturingGrid = 0;
}

LayerInfo* TechLib::createLayer(const std::string &name)
{
    auto layer = lookupLayer(name);
    if (layer != nullptr)
    {
        return layer;
    }   

    layer = new LayerInfo();
    layer->m_name = name;
    
    m_layers.add(name, layer);
    return layer;
}

LayerInfo* TechLib::lookupLayer(const std::string &name) const
{
    return m_layers.lookup(name);
}

SiteInfo* TechLib::createSiteInfo(const std::string &name)
{
    auto siteInfo = lookupSiteInfo(name);
    if (siteInfo != nullptr)
    {
        return siteInfo;
    }   

    siteInfo = new SiteInfo();
    siteInfo->m_name = name;

    m_sites.add(name, siteInfo);
    return siteInfo;
}

SiteInfo* TechLib::lookupSiteInfo(const std::string &name) const
{
    return m_sites.lookup(name);
}
