#pragma once

#include <vector>
#include <optional>

#include "lunacore.h"
#include <QPixmap>
#include <QBrush>

namespace GUI
{

class LayerRenderInfo
{
public:
    LayerRenderInfo() : m_color{QColor("#FFFFFF80")} {}

    LayerRenderInfo(ChipDB::LayerInfo *layer)
    {
        if (layer != nullptr)
        {
            m_layer = *layer;
        }
    }

    QBrush getBrush() const noexcept
    {
        if (!m_pixmap.isNull())
        {
            return QBrush(m_pixmap);
        }
        else
        {
            return QBrush(m_color);
        }
    }

    auto getName() const
    {
        return m_layer.m_name;
    }

    auto getID() const
    {
        return m_layer.m_id;
    }
    
protected:
    ChipDB::LayerInfo m_layer;  ///< copy of layer information
    QPixmap m_pixmap;               
    QColor  m_color;
};



class LayerRenderInfoDB
{
public:
    bool addLayerInfo(const LayerRenderInfo &info);

    std::optional<LayerRenderInfo> getRenderInfo(ChipDB::LayerID id) const;
    
protected:
    std::unordered_map<ChipDB::LayerID, LayerRenderInfo> m_layerInfos;
};


}; // namespace
