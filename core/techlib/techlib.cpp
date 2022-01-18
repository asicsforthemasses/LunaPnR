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

KeyObjPair<LayerInfo> TechLib::createLayer(const std::string &name)
{
    auto layerKeyObj = m_layers.add(std::make_shared<LayerInfo>(name));
    if (layerKeyObj)
    {
        return layerKeyObj.value();
    }

    return KeyObjPair<LayerInfo>();
}

KeyObjPair<LayerInfo> TechLib::lookupLayer(const std::string &name) const
{
    return m_layers[name];
}

KeyObjPair<SiteInfo> TechLib::createSiteInfo(const std::string &name)
{
    auto siteKeyObj = m_sites.add(std::make_shared<SiteInfo>(name));
    if (siteKeyObj)
    {
        return siteKeyObj.value();
    }

    return KeyObjPair<SiteInfo>();
}

KeyObjPair<SiteInfo> TechLib::lookupSiteInfo(const std::string &name) const
{
    return m_sites[name];
}
