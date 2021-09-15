#include "layerrenderinfo.h"

using namespace GUI;

QImage LayerRenderInfo::createTextureImage(QColor col, const QPixmap &pixmap)
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

bool LayerRenderInfo::setTextureFromString(const std::string &pixels, int width, int height)
{
    QImage image(width, height, QImage::Format_Mono);
    image.setColor(0, QColor("#000000").rgb());
    image.setColor(1, QColor("#FFFFFF").rgb());

    if (pixels.size() < (width*height))
    {
        doLog(LOG_ERROR, "Cannot set texture from string: not enough characters\n");
        return false;
    }

    size_t ofs = 0;
    for(int y=0; y<height; y++)
    {
        for(int x=0; x<width; x++)
        {
            if (pixels.at(ofs) != ' ')
            {
                image.setPixel(QPoint(x,y), 1);
            }
            else
            {
                image.setPixel(QPoint(x,y), 0);
            }
            ofs++;
        }
    }

    QPixmap pixmap;
    bool result = pixmap.convertFromImage(image, Qt::NoFormatConversion);
    if (result)
    {
        setTexture(pixmap);
    }

    return result;
}

void LayerRenderInfo::updateTextureWithColor(QColor col)
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

