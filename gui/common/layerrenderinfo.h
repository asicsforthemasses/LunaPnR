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

        void setColor(const QColor &col)
        {
            m_color = col;
            updateColorPixmap();
        }

        QColor getColor() const
        {
            return m_color;
        }

        void setTexture(const QPixmap &pixmap)
        {
            m_patternPixmap = pixmap;
            updateColorPixmap();
        }

        QPixmap getTexture() const
        {
            return m_patternPixmap;
        }

        QPixmap getColorPixmap() const noexcept
        {
            return m_colorPixmap;
        }
        
        void read(const QJsonObject &json);
        void write(QJsonObject &json) const;

    protected:
        void updateColorPixmap();

        QColor          m_color;
        QPixmap         m_patternPixmap;
        QPixmap         m_colorPixmap;
    };

    std::string getName() const noexcept
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


class LayerRenderInfoDB : public ChipDB::NamedStorage<LayerRenderInfo*, true>
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


