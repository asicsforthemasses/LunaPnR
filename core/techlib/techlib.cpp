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

LayerInfo* TechLib::createLayer(const std::string &name)
{
    auto layer = lookup(name);
    if (layer != nullptr)
    {
        return layer;
    }   

    layer = new LayerInfo();
    layer->m_name = name;

    m_layers.add(name, layer);
    return layer;
}

LayerInfo* TechLib::lookup(const std::string &name) const
{
    auto iter = std::find_if(m_layers.begin(), m_layers.end(),
        [name](auto ptr)
        {
            return ptr->m_name == name;
        }
    );

    if (iter == m_layers.end())
        return nullptr;

    return *iter;
}
