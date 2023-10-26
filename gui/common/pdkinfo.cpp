#include "pdkinfo.h"
#include <lunacore.h>

namespace GUI
{

toml::table toToml(const PDKInfo &pdkinfo)
{
    toml::table tbl;
    ChipDB::TOML::insert(tbl, "title", pdkinfo.m_title);
    ChipDB::TOML::insert(tbl, "name", pdkinfo.m_name);
    ChipDB::TOML::insert(tbl, "version", pdkinfo.m_version);
    ChipDB::TOML::insert(tbl, "date", pdkinfo.m_date);
    ChipDB::TOML::insert(tbl, "copyright", pdkinfo.m_copyright);
    ChipDB::TOML::insert(tbl, "description", pdkinfo.m_description);
    ChipDB::TOML::insert(tbl, "layerfile", pdkinfo.m_layerfile);

    ChipDB::TOML::insert(tbl, "lef", pdkinfo.m_lefs);
    ChipDB::TOML::insert(tbl, "lib", pdkinfo.m_libs);
    return std::move(tbl);
}

bool fromToml(std::istream &toml, PDKInfo &pdkinfo)
{
    try
    {    
        auto tbl = toml::parse(toml);

        ChipDB::TOML::retrieve(tbl, "title", pdkinfo.m_title);
        ChipDB::TOML::retrieve(tbl, "name", pdkinfo.m_name);
        ChipDB::TOML::retrieve(tbl, "version", pdkinfo.m_version);
        ChipDB::TOML::retrieve(tbl, "copyright", pdkinfo.m_copyright);
        ChipDB::TOML::retrieve(tbl, "date", pdkinfo.m_date);
        ChipDB::TOML::retrieve(tbl, "description", pdkinfo.m_description);
        ChipDB::TOML::retrieve(tbl, "layerfile", pdkinfo.m_layerfile);

        ChipDB::TOML::retrieve(tbl, "lef", pdkinfo.m_lefs);
        ChipDB::TOML::retrieve(tbl, "lib", pdkinfo.m_libs);

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
