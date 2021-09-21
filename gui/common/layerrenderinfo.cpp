#include <QJsonDocument>
#include "guihelpers.h"
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
    json["color"]  = m_brush.color().name(QColor::HexArgb);
    json["width"]  = m_brush.texture().width();
    json["height"]  = m_brush.texture().height();
    json["pixmap"] = QString::fromStdString(pixmapToString(getPixmap()));
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
        m_layerInfos.clear();

        auto arr = topObj["layers"].toArray();
        for(auto const& obj : arr)
        {
            LayerRenderInfo info;
            info.read(obj.toObject());
            m_layerInfos[info.getName()] = info;
        }
    }
    else
    {
        doLog(LOG_ERROR, "Cannot load JSON layer setup file - JSON does not contain a layers array\n");
        return false;
    }


    doLog(LOG_VERBOSE,"Loaded %d layers from JSON file\n", m_layerInfos.size());
    return true;
}

std::string LayerRenderInfoDB::writeJson() const
{    
    QJsonArray  arr;
    for(auto layer : m_layerInfos)
    {        
        QJsonObject obj;
        layer.second.write(obj);
        arr.append(obj);        
    }
    
    QJsonObject obj;
    obj["layers"] = arr;

    QJsonDocument doc(obj);
    return doc.toJson().toStdString();
}
