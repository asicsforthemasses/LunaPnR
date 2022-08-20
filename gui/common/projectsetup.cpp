#include "projectsetup.h"
#include <sstream>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonDocument>

using namespace GUI;

QJsonArray toJson(const std::vector<std::string> &obj)
{
    QJsonArray arr;
    for(auto const& value : obj)
    {
        arr.append(QString::fromStdString(value));
    }
    return arr;
}

QJsonArray toJson(const ChipDB::Margins64 &obj)
{
    QJsonArray arr;
    arr.append(QJsonValue(static_cast<qint64>(obj.m_left)));
    arr.append(QJsonValue(static_cast<qint64>(obj.m_bottom)));
    arr.append(QJsonValue(static_cast<qint64>(obj.m_right)));
    arr.append(QJsonValue(static_cast<qint64>(obj.m_top)));
    return arr;
}

QJsonArray toJson(const ChipDB::Rect64 &obj)
{
    QJsonArray arr;
    arr.append(QJsonValue(static_cast<qint64>(obj.m_ll.m_x)));
    arr.append(QJsonValue(static_cast<qint64>(obj.m_ll.m_y)));
    arr.append(QJsonValue(static_cast<qint64>(obj.m_ur.m_x)));
    arr.append(QJsonValue(static_cast<qint64>(obj.m_ur.m_y)));
    return arr;
}

QJsonArray toJson(const std::vector<RegionSetup> &obj)
{
    QJsonArray arr;
    for(auto const& value : obj)
    {
        QJsonObject regionObject;
        regionObject["RegionName"] = QString::fromStdString(value.m_regionName);
        regionObject["RegionSite"] = QString::fromStdString(value.m_site);
        regionObject["RegionSize"] = toJson(value.m_regionSize);
        regionObject["RegionHalo"] = toJson(value.m_regionHalo);
        arr.append(regionObject);
    }
    return arr;
}

std::vector<std::string> fromJson(const QJsonArray &arr)
{
    std::vector<std::string> result;
    for(auto const& value : arr)
    {
        result.push_back(value.toString().toStdString());
    }
    return result;
}

ChipDB::Margins64 fromJsonToMargins64(const QJsonArray &arr)
{
    auto left   = arr.at(0).toInt();
    auto bottom = arr.at(1).toInt();
    auto right  = arr.at(2).toInt();
    auto top    = arr.at(3).toInt();

    return ChipDB::Margins64{left, bottom, right, top};
}

ChipDB::Rect64 fromJsonToRect64(const QJsonArray &arr)
{
    ChipDB::Rect64 rect;
    auto left   = arr.at(0).toInt();
    auto bottom = arr.at(1).toInt();
    auto right  = arr.at(2).toInt();
    auto top    = arr.at(3).toInt();

    return ChipDB::Rect64{{left, bottom}, {right,top}};
}

RegionSetup fromJsonToRegionSetup(const QJsonObject &obj)
{
    RegionSetup rs;
    if (obj.contains("RegionName")) rs.m_regionName = obj["RegionName"].toString().toStdString();
    if (obj.contains("RegionSite")) rs.m_site = obj["RegionSite"].toString().toStdString();
    if (obj.contains("RegionSize")) rs.m_regionSize = fromJsonToRect64(obj["RegionSize"].toArray());
    if (obj.contains("RegionHalo")) rs.m_regionHalo = fromJsonToMargins64(obj["RegionHalo"].toArray());
    return rs;
}

std::vector<RegionSetup> fromJsonToRegionArray(const QJsonArray &arr)
{
    std::vector<RegionSetup> result;
    for(auto const& value : arr)
    {
        auto region = fromJsonToRegionSetup(value.toObject());
        result.push_back(region);
    }
    return result;
}


bool ProjectSetup::writeToJSON(std::ostream &os) const
{
    QJsonObject json;

    json["LEF"] = toJson(m_lefFiles);
    json["LIB"] = toJson(m_libFiles);
    json["Timing"] = toJson(m_timingConstraintFiles);
    json["Verilog"] = toJson(m_verilogFiles);
    json["Layers"] = toJson(m_layerFiles);
    json["Regions"] = toJson(m_regions);

    QJsonDocument doc(json);

    auto byteArray = doc.toJson();
    os.write(byteArray.data(), byteArray.size());

    return true;
}

bool ProjectSetup::readFromJSON(std::istream &is)
{
    is.seekg (0, is.end);
    auto length = is.tellg();
    is.seekg (0, is.beg);

    const size_t FourMegabyte = 1024*1024*4;
    if ((length < 0) || (length > FourMegabyte))
    {
        return false;
    }

    QByteArray rawData;
    rawData.resize(length);

    is.read(rawData.data(), length);
    
    QJsonParseError error;
    auto doc = QJsonDocument::fromJson(rawData, &error);
            
    if (doc.isNull())
    {
        std::cerr << "JSON parse error: " << error.errorString().toStdString() << "\n";
        return false;
    }

    if (!doc.isObject())
    {
        return false;
    }

    auto json = doc.object();

    if (json.contains("LEF")) m_lefFiles = fromJson(json["LEF"].toArray());
    if (json.contains("LIB")) m_libFiles = fromJson(json["LIB"].toArray());
    if (json.contains("Verilog")) m_verilogFiles = fromJson(json["Verilog"].toArray());
    if (json.contains("Timing")) m_timingConstraintFiles = fromJson(json["Timing"].toArray());
    if (json.contains("Layers")) m_layerFiles = fromJson(json["Layers"].toArray());
    if (json.contains("Regions")) m_regions = fromJsonToRegionArray(json["Regions"].toArray());

    return true;
}
