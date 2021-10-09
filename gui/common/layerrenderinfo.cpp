#include <QJsonDocument>
#include "guihelpers.h"
#include "layerrenderinfo.h"

using namespace GUI;

void LayerRenderInfo::LayerType::updateColorPixmap()
{
    if (!m_patternPixmap.isNull())
    {
        QPixmap canvas(24,24);
        canvas.fill(Qt::transparent);
        
        QPainter painter(&canvas);
        QImage image(m_patternPixmap.toImage().convertToFormat(QImage::Format_Mono));
        image.setColor(0, QColor("#00000000").rgba());
        image.setColor(1, m_color.rgba());
        //auto colorTexture = QPixmap::fromImage(image);

        QBrush brush;
        brush.setTextureImage(image);
        brush.setColor(m_color);

        std::cout << ((m_patternPixmap.depth() == 1) ? "BITMAP OK\n" : "BITMAP FUCKED\n");

        painter.setPen(Qt::black);
        painter.setBrush(brush);
        painter.drawRect(canvas.rect().adjusted(0,0,-1,-1));
        painter.end();

        m_colorPixmap = canvas;
    }
    else
    {
        QPixmap canvas(24,24);
        canvas.fill(Qt::black);

        QPainter painter(&canvas);
        painter.setBrush(m_color);
        painter.setPen(Qt::black);
        painter.drawRect(canvas.rect().adjusted(0,0,-1,-1));
        painter.end();

        m_colorPixmap = canvas;
    }
}

#if 0
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
        auto textureImage = createTextureImage(col, m_pixmap);
        m_brush.setTextureImage(textureImage);
    }    
}
#endif

void LayerRenderInfo::LayerType::read(const QJsonObject &json)
{
    if (json.contains("color") && json["color"].isString())
    {
        setColor(QColor(json["color"].toString()));
    }

    int w = 0;
    int h = 0;

    if (json.contains("width") && json["width"].isDouble())
    {
        w = json["width"].toInt();
    }

    if (json.contains("height") && json["height"].isDouble())
    {
        h = json["height"].toInt();
    }
    
    if (json.contains("pixmap") && json["pixmap"].isString())
    {
        auto str = json["pixmap"].toString();
        auto pixmap = GUI::createPixmapFromString(str.toStdString(),
            w,h);

        if (pixmap.has_value())
        {
            setTexture(*pixmap);
        }
    }
}

void LayerRenderInfo::LayerType::write(QJsonObject &json) const
{
    json["color"] = m_color.name();
    if (!m_patternPixmap.isNull())
    {
        json["width"]  = m_patternPixmap.width();
        json["height"] = m_patternPixmap.height();
        json["pixmap"] = QString::fromStdString(pixmapToString(m_patternPixmap));
    }
    else
    {
        json["width"]  = 0;
        json["height"] = 0;
        json["pixmap"] = "";
    }
}


void LayerRenderInfo::read(const QJsonObject &json)
{
    if (json.contains("layer") && json["layer"].isString())
    {
        m_layerName = json["layer"].toString().toStdString();
    }

    if (json.contains("types") && json["types"].isArray())
    {
        QJsonArray arr = json["types"].toArray();

        size_t index = 0;
        for(auto &t : m_types)
        {
            t.read(arr[index].toObject());
            index++;
        }
    }
}

void LayerRenderInfo::write(QJsonObject &json) const
{
    QJsonArray arr;
    json["layer"] = QString::fromStdString(m_layerName);    
    
    for(auto const& t : m_types)
    {
        QJsonObject typeObject;
        t.write(typeObject);
        arr.append(typeObject);
    }
    json["types"] = arr;
}

bool LayerRenderInfoDB::readJson(const std::string &txt)
{
    auto doc = QJsonDocument::fromJson(QByteArray(txt.c_str(), txt.size()));
    if (doc.isNull())
    {
        doLog(LOG_ERROR, "Cannot load JSON layer setup file - JSON does not appear to be valid\n");
        return false;
    }

    auto topObj = doc.object();

    if (topObj.contains("layers") && (topObj["layers"].isArray()))
    {
        clear();

        auto arr = topObj["layers"].toArray();
        for(auto const& obj : arr)
        {
            LayerRenderInfo *info = new LayerRenderInfo();
            info->read(obj.toObject());
            add(info->getName(), info);
        }
    }
    else
    {
        doLog(LOG_ERROR, "Cannot load JSON layer setup file - JSON does not contain a layers array\n");
        return false;
    }


    doLog(LOG_VERBOSE,"Loaded %d layers from JSON file\n", m_objects.size());
    return true;
}

std::string LayerRenderInfoDB::writeJson() const
{    
    QJsonArray  arr;
    for(auto layer : m_objects)
    {        
        QJsonObject obj;
        if (layer != nullptr)
        {
            layer->write(obj);
            arr.append(obj);
        }
    }
    
    QJsonObject obj;
    obj["layers"] = arr;

    QJsonDocument doc(obj);
    return doc.toJson().toStdString();
}

LayerRenderInfo* LayerRenderInfoDB::createLayer(const std::string &name)
{
    auto layer = lookup(name);
    if (layer == nullptr)
    {
        layer = new LayerRenderInfo(name);
        add(name, layer);
    }

    return layer;
}
