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

void LayerInfoModel::setLayer(const ChipDB::LayerInfo *layer)
{
    beginResetModel();
    m_rootNode.reset(new LayerInfoNode("Cell", "", m_altColors.getColor()));

    if (layer == nullptr)
    {
        std::cout << "Layer is null\n";
        endResetModel();
        return;
    }

    m_altColors.resetState();

    // generic layer information
    auto layerNode = new LayerInfoNode("Layer", QString::fromStdString(layer->m_name), m_altColors.getColorAndUpdate());
    m_rootNode->addChild(layerNode);

    layerNode->addChild(new LayerInfoNode("ID", layer->m_id, m_altColors.getColorAndUpdate()));
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


// ********************************************************************************
//    LayerTableModel
// ********************************************************************************

LayerTableModel::LayerTableModel(const ChipDB::TechLib *techLib)
{
    m_lightColor = QColor("#F0F0F0");
    m_darkColor  = QColor("#D0D0D0");
    
    setTechLib(techLib);
}

void LayerTableModel::setTechLib(const ChipDB::TechLib *techLib)
{
    beginResetModel();
    m_techLib = techLib;
    endResetModel();
    //synchronizeCellIds(cellLib);
}

#if 0
void LayerTableModel::synchronizeCellIds(const ChipDB::CellLib *cellLib)
{
    if (cellLib == nullptr)
        return;    

    beginResetModel();
    m_cellLib = cellLib;
    m_cellsIndex.resize(m_cellLib->size());
    
    auto cellIter = m_cellLib->cells().begin();
    size_t idx = 0;
    while(cellIter != m_cellLib->cells().end())
    {
        m_cellsIndex[idx] = cellIter.index();
        idx++;
        ++cellIter;
    }
    endResetModel();
}
#endif

int LayerTableModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    if (m_techLib == nullptr)
        return 0;

    return m_techLib->m_layers.size();
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

    if ((m_techLib == nullptr) || (!index.isValid()))
        return v;

    size_t idx = index.row();
    switch(role)
    {
    case Qt::DisplayRole:
    case Qt::EditRole:
        if (idx < rowCount())
        {
            auto layer = m_techLib->m_layers.at(idx);
            if (layer != nullptr)
            {
                switch(index.column())
                {
                case 0: // cell name
                    return QString::fromStdString(layer->m_name);
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
    case Qt::BackgroundColorRole:        
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

const ChipDB::LayerInfo* LayerTableModel::getLayer(int row) const
{
    if (m_techLib == nullptr)
        return nullptr;

    if (row < m_techLib->m_layers.size())
    {
        return m_techLib->m_layers.at(row);
    }
    else
    {
        return nullptr;
    }
}

void LayerTableModel::techLibUpdated()
{
    //synchronizeCellIds(m_cellLib);
}

