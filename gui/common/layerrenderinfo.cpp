#include "layerrenderinfo.h"

using namespace GUI;

QImage LayerRenderInfo::LayerType::createTextureImage(QColor col, const QPixmap &pixmap)
{
    if (!pixmap.isNull())
    {
        if (pixmap.depth() != 1)
        {
            doLog(LOG_ERROR,"LayerRenderInfo::createTextureImage: bitmap depth != 1 -> cannot create brush\n");
            return QImage();
        }

        QImage image(pixmap.toImage().convertToFormat(QImage::Format_Mono));
        image.setColor(0, QColor("#00000000").rgba());
        image.setColor(1, col.rgba());
        return image;
    }
    return QImage();
}

void LayerRenderInfo::LayerType::updateTextureWithColor(QColor col)
{
    if (!m_pixmap.isNull())
    {
        auto textureImage = createTextureImage(m_brush.color(), m_pixmap);
        m_brush.setTextureImage(textureImage);
    }    
}

std::optional<LayerRenderInfo> LayerRenderInfoDB::getRenderInfo(const std::string &layerName) const
{
    auto iter = m_layerInfos.find(layerName);
    if (iter == m_layerInfos.end())
    {
        return std::nullopt;
    }

    return (*iter).second;
}

void LayerRenderInfoDB::setRenderInfo(const std::string &layerName, const LayerRenderInfo &info)
{
    m_layerInfos[layerName] = info;
}

