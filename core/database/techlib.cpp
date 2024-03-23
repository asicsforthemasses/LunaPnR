// SPDX-FileCopyrightText: 2021-2024 Niels Moseley <asicsforthemasses@gmail.com>
//
// SPDX-License-Identifier: GPL-3.0-only

#include "techlib.h"
#include <algorithm>

using namespace ChipDB;

std::string ChipDB::toString(const LayerType &lt)
{
    switch(lt)
    {
    default:
    case LayerType::UNDEFINED:
        return "UNDEFINED";
    case LayerType::ROUTING:
        return "ROUTING";
    case LayerType::CUT:
        return "CUT";
    case LayerType::MASTERSLICE:
        return "MASTERSLICE";
    case LayerType::OVERLAP:
        return "OVERLAP";
    }
}

std::string ChipDB::toString(const LayerDirection &ldir)
{
    switch(ldir)
    {
    default:
    case LayerDirection::UNDEFINED:
        return "UNDEFINED";
    case LayerDirection::HORIZONTAL:
        return "HORIZONTAL";
    case LayerDirection::VERTICAL:
        return "VERTICAL";
    }
}

std::string ChipDB::toString(const SiteClass &cls)
{
    switch(cls)
    {
    default:
    case SiteClass::UNDEFINED:
        return "UNDEFINED";
    case SiteClass::PAD:
        return "PAD";
    case SiteClass::CORE:
        return "CORE";
    }
}

void TechLib::clear()
{
    m_layers.clear();
    m_sites.clear();
    m_manufacturingGrid = 1;
}

KeyObjPair<LayerInfo> TechLib::createLayer(const std::string &name)
{
    auto layerKeyObj = m_layers.add(std::make_shared<LayerInfo>(name));
    if (layerKeyObj)
    {
        return layerKeyObj.value();
    }

    return KeyObjPair<LayerInfo>{};
}

KeyObjPair<LayerInfo> TechLib::lookupLayer(const std::string &name) const
{
    return m_layers[name];
}

std::shared_ptr<LayerInfo> TechLib::lookupLayer(const ChipDB::ObjectKey key) const
{
    return m_layers.at(key);
}

KeyObjPair<SiteInfo> TechLib::createSiteInfo(const std::string &name)
{
    auto siteKeyObj = m_sites.add(std::make_shared<SiteInfo>(name));
    if (siteKeyObj)
    {
        return siteKeyObj.value();
    }

    return KeyObjPair<SiteInfo>{};
}

KeyObjPair<SiteInfo> TechLib::lookupSiteInfo(const std::string &name) const
{
    return m_sites[name];
}

std::shared_ptr<SiteInfo> TechLib::lookupSiteInfo(const ChipDB::ObjectKey key) const
{
    return m_sites.at(key);
}

void TechLib::addLayerListener(ChipDB::INamedStorageListener *listener)
{
    m_layers.addListener(listener);
}

void TechLib::removeLayerListener(ChipDB::INamedStorageListener *listener)
{
    m_layers.removeListener(listener);
}

void TechLib::addSiteListener(ChipDB::INamedStorageListener *listener)
{
    m_sites.addListener(listener);
}

void TechLib::removeSiteListener(ChipDB::INamedStorageListener *listener)
{
    m_sites.removeListener(listener);
}

#if 0
void TechLib::contentsChanged() const
{
    m_sites.contentsChanged();
    m_layers.contentsChanged();
}
#endif