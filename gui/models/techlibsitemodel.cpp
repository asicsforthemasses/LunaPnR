// SPDX-FileCopyrightText: 2021-2023 Niels Moseley <asicsforthemasses@gmail.com>
//
// SPDX-License-Identifier: GPL-3.0-only

#include "techlibsitemodel.h"
#include "common/guihelpers.h"

using namespace GUI;

//
// TODO: implement a base or template class for a node
//       so we get max. code re-use
//

// ********************************************************************************
//    SiteInfoNode
// ********************************************************************************

SiteInfoNode::SiteInfoNode(const QString &valueName, const QVariant &value,
    const QColor &bkColor)
    : NodeBase(valueName, value)
{
    m_bkColors[0] = bkColor;
    m_bkColors[1] = bkColor;
}

// ********************************************************************************
//    SiteInfoModel
// ********************************************************************************

SiteInfoModel::SiteInfoModel()
{
    auto lightColor = QColor("#F0F0F0");
    auto darkColor  = QColor("#D0D0D0");
    m_altColors.setColors(lightColor, darkColor);
}

SiteInfoModel::~SiteInfoModel()
{
}

void SiteInfoModel::setSite(std::shared_ptr<ChipDB::SiteInfo> site)
{
    beginResetModel();
    m_rootNode.reset(new SiteInfoNode("Site", "", m_altColors.getColor()));

    if (!site)
    {
        //std::cout << "Layer is null\n";
        endResetModel();
        return;
    }

    m_altColors.resetState();

    // generic layer information
    auto layerNode = new SiteInfoNode("Site name", QString::fromStdString(site->name()), m_altColors.getColorAndUpdate());
    m_rootNode->addChild(layerNode);

    layerNode->addChild(new SiteInfoNode("Class", QString::fromStdString(toString(site->m_class)), m_altColors.getColorAndUpdate()));
    layerNode->addChild(new SiteInfoNode("Size (nm)", QString::asprintf("%ld,%ld", site->m_size.m_x, site->m_size.m_y), m_altColors.getColorAndUpdate()));
    layerNode->addChild(new SiteInfoNode("Symmetry", QString::fromStdString(toString(site->m_symmetry)), m_altColors.getColorAndUpdate()));

    endResetModel();
}

void SiteInfoModel::notify(ChipDB::ObjectKey index, ChipDB::INamedStorageListener::NotificationType t)
{
    if (t == ChipDB::INamedStorageListener::NotificationType::REMOVE)
    {
        setSite(nullptr);
    }
}

// ********************************************************************************
//    SiteTableModel
// ********************************************************************************

SiteTableModel::SiteTableModel(std::shared_ptr<ChipDB::TechLib> techLib) : m_techLib(nullptr)
{
    m_lightColor = QColor("#F0F0F0");
    m_darkColor  = QColor("#D0D0D0");
    
    setTechLib(techLib);
}

SiteTableModel::~SiteTableModel()
{
    if (m_techLib)
    {
        m_techLib->removeSiteListener(this);
    }
}

void SiteTableModel::setTechLib(std::shared_ptr<ChipDB::TechLib> techLib)
{
    if (m_techLib)
    {
        m_techLib->removeSiteListener(this);
    }

    beginResetModel();
    m_techLib = techLib;
    if (m_techLib)
    {
        m_techLib->addSiteListener(this);
    }

    endResetModel();
}

void SiteTableModel::notify(ChipDB::ObjectKey index, ChipDB::INamedStorageListener::NotificationType t)
{
    beginResetModel();
    endResetModel();
}

int SiteTableModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    if (!m_techLib)
        return 0;

    return m_techLib->getNumberOfSites();
}

/** return the number of columns in the table */
int SiteTableModel::columnCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return 2;
}

QVariant SiteTableModel::data(const QModelIndex &index, int role) const
{
    QVariant v;

    if ((!m_techLib) || (!index.isValid()))
        return v;

    size_t idx = index.row();
    switch(role)
    {
    case Qt::DisplayRole:
    case Qt::EditRole:
        if (idx < rowCount())
        {
            //FIXME: use keys
            auto site = m_techLib->sites().at(idx);
            if (site != nullptr)
            {
                switch(index.column())
                {
                case 0: // site name
                    return QString::fromStdString(site->name());
                case 1: // site class
                    return QString::fromStdString(ChipDB::toString(site->m_class));
                default:
                    return QString("");
                    //return QString::fromStdString(ChipDB::toString(layer->m_));
                }
            }
                
            else
                return QString("(null)");
        }
        break;
    case Qt::BackgroundRole:        
        if (index.row() % 2)
            return m_darkColor;
        else
            return m_lightColor;

        break;    
    }

    return v;
}

QVariant SiteTableModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    const std::array<const char *,2> headerNames= 
    {
        "Name","Class"
    };

    QVariant v;
    if (orientation == Qt::Horizontal)
    {
        switch(role)
        {
        case Qt::DisplayRole:
            if (section < headerNames.size())
                return QString(headerNames[section]);
            break;
        }
    }

    return v;
}

Qt::ItemFlags SiteTableModel::flags(const QModelIndex &index) const
{
    return Qt::ItemIsEnabled | Qt::ItemIsSelectable;
}

const std::shared_ptr<ChipDB::SiteInfo> SiteTableModel::getSite(int row) const
{
    if (m_techLib == nullptr)
        return nullptr;

    if (row < m_techLib->getNumberOfSites())
    {
        return m_techLib->sites().at(row);
    }
    else
    {
        return nullptr;
    }
}

