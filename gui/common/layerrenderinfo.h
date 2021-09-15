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
    LayerRenderInfo()
    {
        m_brush.setColor(QColor("#80FFFFFF"));
        m_brush.setStyle(Qt::SolidPattern);
    }

    LayerRenderInfo(const std::string &layerName) 
    {
        m_brush.setColor(QColor("#80FFFFFF"));
        m_brush.setStyle(Qt::SolidPattern);
        m_layerName = layerName;
    }

    void setColor(const QColor &col)
    {
        m_brush.setColor(col);
        updateTextureWithColor(col);
    }

    bool setTextureFromString(const std::string &pixels, int width, int height);

    void setTexture(const QPixmap &pixmap)
    {
        m_pixmap = pixmap;
        updateTextureWithColor(m_brush.color());
    }

    QBrush getBrush() const noexcept
    {
        return m_brush;
    }

    QPixmap getPixmap() const noexcept
    {
        return m_pixmap;
    }

    std::string getName() const noexcept
    {
        return m_layerName;
    }
    
protected:
    QImage createTextureImage(QColor col, const QPixmap &pixmap);
    void   updateTextureWithColor(QColor col);

    std::string     m_layerName;
    QBrush          m_brush;
    QPixmap         m_pixmap;
};



class LayerRenderInfoDB
{
public:
    std::optional<LayerRenderInfo> getRenderInfo(const std::string &layerName) const;
    void setRenderInfo(const std::string &layerName, const LayerRenderInfo &info);

protected:
    std::unordered_map<std::string, LayerRenderInfo> m_layerInfos;
};


}; // namespace
