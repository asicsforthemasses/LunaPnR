// SPDX-FileCopyrightText: 2021-2024 Niels Moseley <asicsforthemasses@gmail.com>
//
// SPDX-License-Identifier: GPL-3.0-only

#include <QJsonDocument>
#include "guihelpers.h"
#include "layerrenderinfo.h"

using namespace GUI;

QPixmap LayerRenderInfo::LayerType::createToolbarPixmap(const QSize &s, const QColor &background)
{
    QPixmap pixmap(s);
    pixmap.fill(background.rgba());

    QPainter painter(&pixmap);
    QBrush brush(m_colorPixmap);
    painter.setBrush(brush);
    painter.setPen(Qt::black);
    painter.drawRect(pixmap.rect().adjusted(0,0,-1,-1));
    painter.end();
    return pixmap;
}

void LayerRenderInfo::LayerType::updateColorPixmap()
{
    if (!m_patternPixmap.isNull())
    {
        QPixmap canvas(m_patternPixmap.size());
        canvas.fill(Qt::transparent);

        QPainter painter(&canvas);
        QImage image(m_patternPixmap.toImage().convertToFormat(QImage::Format_Mono));
        image.setColor(0, QColor("#00000000").rgba());
        image.setColor(1, m_color.rgba());

        QBrush brush;
        brush.setTextureImage(image);
        brush.setColor(m_color);

        painter.fillRect(canvas.rect(), brush);
        painter.end();

        m_colorPixmap = canvas;
    }
    else
    {
        QPixmap canvas(24,24);
        canvas.fill(Qt::black);

        QPainter painter(&canvas);
        painter.fillRect(canvas.rect(), m_color);
        painter.end();

        m_colorPixmap = canvas;
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
        Logging::logError("Cannot load JSON layer setup file - JSON does not appear to be valid\n");
        return false;
    }

    auto topObj = doc.object();

    if (topObj.contains("layers") && (topObj["layers"].isArray()))
    {
        clear();

        auto arr = topObj["layers"].toArray();
        for(auto const& obj : arr)
        {
            auto layerObj = std::make_shared<LayerRenderInfo>();
            layerObj->read(obj.toObject());

            auto layerOpt = add(layerObj);
            if (!layerOpt.has_value())
            {
                Logging::logError("Cannot add layer %s to database - already exists!\n", layerObj->name().c_str());
            }
        }
    }
    else
    {
        Logging::logError("Cannot load JSON layer setup file - JSON does not contain a layers array\n");
        return false;
    }

    notifyAll();
    Logging::logVerbose("Loaded %d layers from JSON file\n", m_objects.size());
    return true;
}

std::string LayerRenderInfoDB::writeJson() const
{
    QJsonArray  arr;
    for(auto layer : m_objects)
    {
        QJsonObject obj;
        if (layer.second)
        {
            layer.second->write(obj);
            arr.append(obj);
        }
    }

    QJsonObject obj;
    obj["layers"] = arr;

    QJsonDocument doc(obj);
    return doc.toJson().toStdString();
}

ChipDB::KeyObjPair<GUI::LayerRenderInfo> LayerRenderInfoDB::createLayer(const std::string &name)
{
    auto layerObjKeyPair = this->findObject(name);
    if (!layerObjKeyPair.isValid())
    {
        auto layerObjKeyPairOpt = add(std::make_shared<LayerRenderInfo>(name));
        if (layerObjKeyPairOpt.has_value())
        {
            layerObjKeyPair = layerObjKeyPairOpt.value();
        }
        else
        {
            // internal error?
            Logging::logError("LayerRenderInfoDB::createLayer internal error\n");
            return ChipDB::KeyObjPair<GUI::LayerRenderInfo>();
        }
    }

    return layerObjKeyPair;
}
