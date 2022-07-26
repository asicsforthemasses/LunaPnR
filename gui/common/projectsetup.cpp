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

std::vector<std::string> fromJson(const QJsonArray &arr)
{
    std::vector<std::string> result;
    for(auto const& value : arr)
    {
        result.push_back(value.toString().toStdString());
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

    return true;
}
