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

ChipDB::PinInfo CellPinListModel::getPinInfo(int row) const
{
    if (m_cell == nullptr)
        return ChipDB::PinInfo();

    if (row < m_cell->m_pins.size())
    {
        return m_cell->m_pins.at(row);
    }
    else
    {
        return ChipDB::PinInfo();
    }
}
