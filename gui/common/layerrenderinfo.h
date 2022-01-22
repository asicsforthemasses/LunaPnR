#pragma once

#include <vector>
#include <optional>
#include <array>

#include "lunacore.h"
#include <QPixmap>
#include <QBrush>

#include <QJsonObject>
#include <QJsonArray>

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
        LayerType() : m_color(Qt::black)
        {
            updateColorPixmap();
        }

        LayerType(const LayerType &) = default;

        /** set the color of the layer hatch pattern */
        void setColor(const QColor &col)
        {
            m_color = col;
            updateColorPixmap();
        }


        /** get the color of the layer hatch pattern */
        QColor getColor() const
        {
            return m_color;
        }

        /** set the texture for the hatch pattern.
         *  the pixmap must be one bit per pixel
        */
        void setTexture(const QPixmap &pixmap)
        {
            if (pixmap.depth() == 1)
            {
                m_patternPixmap = pixmap;
            }
            updateColorPixmap();
        }

        /** get the texture for the hatch pattern.
         *  this is a one-bit-per-pixel pixmap.
         * 
        */
        QPixmap getTexture() const
        {
            return m_patternPixmap;
        }

        /** return a transparent color pixmap.
         *  this is meant to be used to draw
         *  cell geometry.
        */
        QPixmap getColorPixmap() const noexcept
        {
            return m_colorPixmap;
        }
        
        /** create a pixmap of the given size, filled with a background color and
         *  apply the color hatch pattern. This is meant for layer tool / list displays
        */
        QPixmap createToolbarPixmap(const QSize &s, const QColor &background);

        void read(const QJsonObject &json);
        void write(QJsonObject &json) const;

    protected:
        void updateColorPixmap();

        QColor          m_color;
        QPixmap         m_patternPixmap;
        QPixmap         m_colorPixmap;
    };

    std::string name() const noexcept
    {
        return m_layerName;
    }

    void setName(const std::string &layerName)
    {
        m_layerName = layerName;
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

    void read(const QJsonObject &json);
    void write(QJsonObject &json) const;

protected:
    std::array<LayerType, 2>    m_types;
    std::string                 m_layerName;
};


class LayerRenderInfoDB : public ChipDB::NamedStorage<LayerRenderInfo>
{
public:

    /** create a layer with the given name and return a pointer to the new layer.
     *  if it already exists, a pointer to the existing layer is returned.
    */
    LayerRenderInfo* createLayer(const std::string &name);

    bool readJson(const std::string &txt);
    std::string writeJson() const;
};

}; // namespace


