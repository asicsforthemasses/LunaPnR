// SPDX-FileCopyrightText: 2021-2025 Niels Moseley <asicsforthemasses@gmail.com>
//
// SPDX-License-Identifier: GPL-3.0-only

#include "techlibinfomodel.h"
#include "common/guihelpers.h"

using namespace GUI;

//
// TODO: implement a base or template class for a node
//       so we get max. code re-use
//

// ********************************************************************************
//    LayerInfoNode
// ********************************************************************************

LayerInfoNode::LayerInfoNode(const QString &valueName, const QVariant &value,
    const QColor &bkColor)
    : NodeBase(valueName, value)
{
    m_bkColors[0] = bkColor;
    m_bkColors[1] = bkColor;
}

// ********************************************************************************
//    LayerInfoModel
// ********************************************************************************

LayerInfoModel::LayerInfoModel()
{
    auto lightColor = QColor("#F0F0F0");
    auto darkColor  = QColor("#D0D0D0");
    m_altColors.setColors(lightColor, darkColor);
}

LayerInfoModel::~LayerInfoModel()
{
}

void LayerInfoModel::setLayer(std::shared_ptr<ChipDB::LayerInfo> layer)
{
    beginResetModel();
    m_rootNode.reset(new LayerInfoNode("Layer", "", m_altColors.getColor()));

    if (!layer)
    {
        //std::cout << "Layer is null\n";
        endResetModel();
        return;
    }

    m_altColors.resetState();

    // generic layer information
    auto layerNode = new LayerInfoNode("Layer name", QString::fromStdString(layer->name()), m_altColors.getColorAndUpdate());
    m_rootNode->addChild(layerNode);

    //layerNode->addChild(new LayerInfoNode("ID", layer->m_id, m_altColors.getColorAndUpdate()));
    layerNode->addChild(new LayerInfoNode("Type", QString::fromStdString(toString(layer->m_type)), m_altColors.getColorAndUpdate()));
    layerNode->addChild(new LayerInfoNode("Direction", QString::fromStdString(toString(layer->m_dir)), m_altColors.getColorAndUpdate()));
    layerNode->addChild(new LayerInfoNode("Thickness", layer->m_thickness, m_altColors.getColorAndUpdate()));
    layerNode->addChild(new LayerInfoNode("Width (nm)", layer->m_width, m_altColors.getColorAndUpdate()));
    layerNode->addChild(new LayerInfoNode("Max width (nm)", layer->m_maxWidth, m_altColors.getColorAndUpdate()));
    layerNode->addChild(new LayerInfoNode("Min area (um²)", layer->m_minArea, m_altColors.getColorAndUpdate()));
    layerNode->addChild(new LayerInfoNode("Spacing (nm)", layer->m_spacing, m_altColors.getColorAndUpdate()));
    layerNode->addChild(new LayerInfoNode("Pitch (nm)", QString::asprintf("%ld,%ld", layer->m_pitch.m_x, layer->m_pitch.m_y), m_altColors.getColorAndUpdate() ));
    layerNode->addChild(new LayerInfoNode("Offset (nm)", QString::asprintf("%ld,%ld", layer->m_offset.m_x, layer->m_offset.m_y), m_altColors.getColorAndUpdate() ));
    layerNode->addChild(new LayerInfoNode("Resistance (ohm/sq)", layer->m_resistance, m_altColors.getColorAndUpdate()));
    layerNode->addChild(new LayerInfoNode("Capacitance (F/um²)", layer->m_capacitance, m_altColors.getColorAndUpdate()));

    endResetModel();
}

void LayerInfoModel::notify(ChipDB::ObjectKey index, ChipDB::INamedStorageListener::NotificationType t)
{
    if (t == ChipDB::INamedStorageListener::NotificationType::REMOVE)
    {
        setLayer(nullptr);
    }
}

// ********************************************************************************
//    LayerTableModel
// ********************************************************************************

LayerTableModel::LayerTableModel(std::shared_ptr<ChipDB::TechLib> techLib) : m_techLib(nullptr)
{
    m_lightColor = QColor("#F0F0F0");
    m_darkColor  = QColor("#D0D0D0");

    setTechLib(techLib);
}

LayerTableModel::~LayerTableModel()
{
    if (m_techLib)
    {
        m_techLib->removeLayerListener(this);
    }
}

void LayerTableModel::setTechLib(std::shared_ptr<ChipDB::TechLib> techLib)
{
    if (m_techLib)
    {
        m_techLib->removeLayerListener(this);
    }

    beginResetModel();
    m_techLib = techLib;
    if (m_techLib)
    {
        m_techLib->addLayerListener(this);
    }

    endResetModel();
}

void LayerTableModel::notify(ChipDB::ObjectKey index, ChipDB::INamedStorageListener::NotificationType t)
{
    beginResetModel();
    endResetModel();
}

int LayerTableModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    if (!m_techLib)
        return 0;

    return m_techLib->getNumberOfLayers();
}

/** return the number of columns in the table */
int LayerTableModel::columnCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return 2;
}

QVariant LayerTableModel::data(const QModelIndex &index, int role) const
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
            auto layer = m_techLib->layers().at(idx);
            if (layer != nullptr)
            {
                switch(index.column())
                {
                case 0: // cell name
                    return QString::fromStdString(layer->name());
                case 1: // cell class
                    //return QString("");
                    return QString::fromStdString(ChipDB::toString(layer->m_type));
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

QVariant LayerTableModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    const std::array<const char *,2> headerNames=
    {
        "Name","Type"
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

Qt::ItemFlags LayerTableModel::flags(const QModelIndex &index) const
{
    return Qt::ItemIsEnabled | Qt::ItemIsSelectable;
}

const std::shared_ptr<ChipDB::LayerInfo> LayerTableModel::getLayer(int row) const
{
    if (m_techLib == nullptr)
        return nullptr;

    if (row < m_techLib->getNumberOfLayers())
    {
        return m_techLib->layers().at(row);
    }
    else
    {
        return nullptr;
    }
}

