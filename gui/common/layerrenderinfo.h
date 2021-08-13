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
    LayerRenderInfo() : m_color(QColor("#80FFFFFF")), m_layerID(-1) {}

    LayerRenderInfo(const std::string &layerName, ChipDB::LayerID id) : m_color{QColor("#80FFFFFF")} 
    {
        m_layerName = layerName;
        m_layerID   = id;
    }

    void setColor(const QColor &col)
    {
        m_color = col;
    }

    void setPixmap(const QPixmap &pixmap)
    {
        m_pixmap = pixmap;
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

    std::string getName() const noexcept
    {
        return m_layerName;
    }

    ChipDB::LayerID getID() const noexcept
    {
        return m_layerID;
    }
    
protected:
    std::string     m_layerName;
    ChipDB::LayerID m_layerID;
    QPixmap         m_pixmap;               
    QColor          m_color;
};



class LayerRenderInfoDB
{
public:
    bool addLayerInfo(const LayerRenderInfo &info);

    std::optional<LayerRenderInfo> getRenderInfo(ChipDB::LayerID id) const;
    bool setRenderInfo(ChipDB::LayerID id, const LayerRenderInfo &info);

protected:
    std::unordered_map<ChipDB::LayerID, LayerRenderInfo> m_layerInfos;
};


}; // namespace
