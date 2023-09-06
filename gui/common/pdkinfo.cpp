#include "pdkinfo.h"

namespace GUI
{

toml::table toToml(const PDKInfo &pdkinfo)
{
    toml::table tbl;
    tbl.insert("title", pdkinfo.m_title);
    tbl.insert("name", pdkinfo.m_name);
    tbl.insert("version", pdkinfo.m_version);
    tbl.insert("date", pdkinfo.m_date);
    tbl.insert("copyright", pdkinfo.m_copyright);
    tbl.insert("description", pdkinfo.m_description);
    tbl.insert("layerfile", pdkinfo.m_layerfile);

    toml::array lefArr;
    lefArr.insert(lefArr.begin(), pdkinfo.m_lefs.begin(), pdkinfo.m_lefs.end());
    tbl.insert("lefs", lefArr);

    toml::array libArr;
    libArr.insert(libArr.begin(), pdkinfo.m_libs.begin(), pdkinfo.m_libs.end());
    tbl.insert("libs", libArr);

    return std::move(tbl);
}

bool fromToml(std::istream &toml, PDKInfo &pdkinfo)
{
    try
    {    
        auto tbl = toml::parse(toml);

        pdkinfo.m_title        = tbl["title"].value_or("");
        pdkinfo.m_name         = tbl["name"].value_or("");
        pdkinfo.m_version      = tbl["version"].value_or("");
        pdkinfo.m_description  = tbl["description"].value_or("");
        pdkinfo.m_date         = tbl["date"].value_or("");
        pdkinfo.m_copyright    = tbl["copyright"].value_or("");
        pdkinfo.m_layerfile    = tbl["layerfile"].value_or("");

        for(int idx = 0; idx < !!tbl["lef"]; idx++)
        {
            auto lef = tbl["lef"][idx].value<std::string>().value();
            pdkinfo.m_lefs.emplace_back(lef);
        }

        for(int idx = 0; idx < !!tbl["lib"]; idx++)
        {
            auto lib = tbl["lib"][idx].value<std::string>().value();
            pdkinfo.m_libs.emplace_back(lib);
        }

        if (pdkinfo.m_title.empty())
        {
            return false;
        }

        if (pdkinfo.m_name.empty())
        {
            return false;
        }

        if (pdkinfo.m_version.empty())
        {
            return false;
        }

        if (pdkinfo.m_libs.empty())
        {
            return false;
        }

        if (pdkinfo.m_lefs.empty())
        {
            return false;
        }
    }
    catch (const toml::parse_error& err)
    {
        return false;
    }

    return true;
}

};
