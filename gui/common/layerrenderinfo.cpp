#include "layerrenderinfo.h"

using namespace GUI;


bool LayerRenderInfoDB::addLayerInfo(const LayerRenderInfo &info)
{
    if (info.getID() < 0)
    {
        // layer does not have a valid ID
        return false;
    }

    m_layerInfos[info.getID()] = info;
    return true;
}

std::optional<LayerRenderInfo> LayerRenderInfoDB::getRenderInfo(ChipDB::LayerID id) const
{
    auto iter = m_layerInfos.find(id);
    if (iter == m_layerInfos.end())
    {
        return std::nullopt;
    }

    return (*iter).second;
}
