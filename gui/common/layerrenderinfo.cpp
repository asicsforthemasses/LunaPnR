#include "layerrenderinfo.h"

using namespace GUI;


bool LayerRenderInfoDB::addLayerInfo(const LayerRenderInfo &info)
{
    return setRenderInfo(info.getID(), info);
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

bool LayerRenderInfoDB::setRenderInfo(ChipDB::LayerID id, const LayerRenderInfo &info)
{
    if (id < 0)
    {
        // not a valid ID
        return false;
    }

    m_layerInfos[info.getID()] = info;
    return true;
}
