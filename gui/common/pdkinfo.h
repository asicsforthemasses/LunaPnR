#pragma once
#include <iostream>
#include <string>
#include <vector>
#include <toml++/toml.h>

namespace GUI
{

struct PDKInfo
{
    std::string m_title;
    std::string m_name;
    std::string m_version;
    std::string m_date;
    std::string m_copyright;
    std::string m_description;
    std::string m_layerfile;
    std::vector<std::string> m_lefs;
    std::vector<std::string> m_libs;    
};

toml::table toToml(const PDKInfo &pdkinfo);
PDKInfo fromToml(std::istream &is);

};
