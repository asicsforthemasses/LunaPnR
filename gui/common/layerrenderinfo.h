#pragma once

#include <vector>
#include <optional>
#include <array>

#include "lunacore.h"
#include <QPixmap>
#include <QBrush>

namespace GUI
{

class LayerRenderInfo
{
public:
    LayerRenderInfo(const std::string &layerName) : m_layerName(layerName)
    {
    }

    LayerRenderInfo() : m_layerName("INVALID")
    {
    }

    class LayerType
    {
    public:
        LayerType()
        {
            m_brush.setColor(QColor("#FFFFFF"));
            m_brush.setStyle(Qt::SolidPattern);
        }

        LayerType(const LayerType &) = default;

        void setColor(const QColor &col)
        {
            m_brush.setColor(col);
            updateTextureWithColor(col);
        }

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
        
    protected:
        QImage createTextureImage(QColor col, const QPixmap &pixmap);
        void   updateTextureWithColor(QColor col);

        
        QBrush          m_brush;
        QPixmap         m_pixmap;
    };

    std::string getName() const noexcept
    {
        return m_layerName;
    }

    auto& obstruction() const
    {
        return m_types.at(1);
    }

    auto& routing() const
    {
        return m_types.at(0);
    }

    auto& obstruction()
    {
        return m_types.at(1);
    }

    auto& routing()
    {
        return m_types.at(0);
    }

protected:
    std::array<LayerType, 2>    m_types;
    std::string                 m_layerName;
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
