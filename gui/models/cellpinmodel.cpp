#include <array>
#include <QString>
#include <QColor>
#include "cellpinmodel.h"

using namespace GUI;

// ********************************************************************************
//    CellPinListModel
// ********************************************************************************

CellPinListModel::CellPinListModel(const ChipDB::Cell *cell)
{
    setCell(cell);
}

void CellPinListModel::setCell(const ChipDB::Cell *cell)
{
    beginResetModel();
    m_cell = cell;
    endResetModel();
}

#if 0
void CellPinListModel::synchronizeCellIds(const ChipDB::CellLib *cellLib)
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

int CellPinListModel::rowCount(const QModelIndex &parent) const
{
    if (m_cell == nullptr)
        return 0;

    return m_cell->m_pins.size();
}

QVariant CellPinListModel::data(const QModelIndex &index, int role) const
{
    QVariant v;

    if ((m_cell == nullptr) || (!index.isValid()))
        return v;

    size_t idx = index.row();
    switch(role)
    {
    case Qt::DisplayRole:
    case Qt::EditRole:
        if (idx < rowCount())
        {
            auto const& pin = m_cell->m_pins.at(idx);
            return QString::fromStdString(pin.m_name);
        }
        break;
    case Qt::BackgroundColorRole:
        if (index.row() % 2)
            return QColor(0xD0D0D0);
        else
            return QColor(0xF0F0F0);

        break;    
    }

    return v;
}

QVariant CellPinListModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    return "";
}

Qt::ItemFlags CellPinListModel::flags(const QModelIndex &index) const
{
    return Qt::ItemIsEnabled | Qt::ItemIsSelectable;
}

const ChipDB::PinInfo* CellPinListModel::getPinInfo(int row)
{
    if (m_cell == nullptr)
        return nullptr;

    if (row < m_cell->m_pins.size())
    {
        return m_cell->m_pins[row];
    }
    else
    {
        return nullptr;
    }
}




// ********************************************************************************
//    PinInfoTableModel
// ********************************************************************************

PinInfoTableModel::PinInfoTableModel(const ChipDB::PinInfo *pinInfo)
{
    setPinInfo(pinInfo);
}

void PinInfoTableModel::setPinInfo(const ChipDB::PinInfo *pinInfo)
{
    beginResetModel();
    m_pinInfo = pinInfo;
    endResetModel();
}

int PinInfoTableModel::rowCount(const QModelIndex &parent) const
{
    if (m_pinInfo == nullptr)
        return 0;

    return 8;
}

int PinInfoTableModel::columnCount(const QModelIndex &parent) const
{
    if (m_pinInfo == nullptr)
        return 0;

    return 2;
}

QVariant PinInfoTableModel::data(const QModelIndex &index, int role) const
{
    QVariant v;

    if ((m_pinInfo == nullptr) || (!index.isValid()))
        return v;

    size_t idx = index.row();
    switch(role)
    {
    case Qt::DisplayRole:
    case Qt::EditRole:
        if (index.column() == 0)
        {
            switch(idx)
            {
            case 0:
                return "Name";
            case 1:
                return "IO type";
            case 2:
                return "Function";
            case 3:
                return "Max fanout";
            case 4:
                return "Cap (pF)";
            case 5:
                return "Max cap (pF)";
            default:
                return "";
            }
        }
        else
        {
            switch(idx)
            {
            case 0:
                return QString::fromStdString(m_pinInfo->m_name);
            case 1:
                return QString::fromStdString(ChipDB::toString(m_pinInfo->m_iotype));
            case 2:
                return QString::fromStdString(m_pinInfo->m_function);
            case 3:
                return m_pinInfo->m_maxFanOut;
            case 4:
                return m_pinInfo->m_cap * 1.0e12;
            case 5:
                return m_pinInfo->m_maxCap * 1.0e12;
            default:
                return "";
            }            
        }
        break;
    case Qt::BackgroundColorRole:
        if (index.row() % 2)
            return QColor(0xD0D0D0);
        else
            return QColor(0xF0F0F0);

        break;    
    }

    return v;
}

QVariant PinInfoTableModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    const std::array<const char *,2> headerNames= 
    {
        "Property","Value"
    };

    QVariant v;
    if (orientation == Qt::Horizontal)
    {
        switch(role)
        {
        case Qt::DisplayRole:
            return QString(headerNames[section]);
        default:
            break;
        }
    }

    return v;
}

Qt::ItemFlags PinInfoTableModel::flags(const QModelIndex &index) const
{
    return Qt::ItemIsEnabled | Qt::ItemIsSelectable;
}
