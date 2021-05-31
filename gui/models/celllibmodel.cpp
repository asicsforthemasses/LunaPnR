#include <array>
#include <QString>
#include <QColor>
#include "celllibmodel.h"

using namespace GUI;

// ********************************************************************************
//    CellLibListModel
// ********************************************************************************

CellLibListModel::CellLibListModel(const ChipDB::CellLib *cellLib)
{
    setCellLib(cellLib);    
}

void CellLibListModel::setCellLib(const ChipDB::CellLib *cellLib)
{
    beginResetModel();
    m_cellLib = cellLib;
    endResetModel();
    //synchronizeCellIds(cellLib);
}

#if 0
void CellLibListModel::synchronizeCellIds(const ChipDB::CellLib *cellLib)
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

int CellLibListModel::rowCount(const QModelIndex &parent) const
{
    if (m_cellLib == nullptr)
        return 0;

    return m_cellLib->size();
}

QVariant CellLibListModel::data(const QModelIndex &index, int role) const
{
    QVariant v;

    if ((m_cellLib == nullptr) || (!index.isValid()))
        return v;

    size_t idx = index.row();
    switch(role)
    {
    case Qt::DisplayRole:
    case Qt::EditRole:
        if (idx < rowCount())
        {
            auto cell = m_cellLib->m_cells.at(idx);
            if (cell != nullptr)
                return QString::fromStdString(cell->m_name);
            else
                return QString("(null)");
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

QVariant CellLibListModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    return "";
}

Qt::ItemFlags CellLibListModel::flags(const QModelIndex &index) const
{
    return Qt::ItemIsEnabled | Qt::ItemIsSelectable;
}

const ChipDB::Cell* CellLibListModel::getCell(int row) const
{
    if (m_cellLib == nullptr)
        return nullptr;

    if (row < m_cellLib->size())
    {
        return m_cellLib->m_cells.at(row);
    }
    else
    {
        return nullptr;
    }
}

void CellLibListModel::cellLibUpdated()
{
    //synchronizeCellIds(m_cellLib);
}

// ********************************************************************************
//    CellLibTableModel
// ********************************************************************************

CellLibTableModel::CellLibTableModel(const ChipDB::CellLib *cellLib)
{
    setCellLib(cellLib);
}

void CellLibTableModel::setCellLib(const ChipDB::CellLib *cellLib)
{
    beginResetModel();
    m_cellLib = cellLib;
    endResetModel();
    //synchronizeCellIds(cellLib);
}

#if 0
void CellLibTableModel::synchronizeCellIds(const ChipDB::CellLib *cellLib)
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

int CellLibTableModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    if (m_cellLib == nullptr)
        return 0;

    return m_cellLib->size();
}

/** return the number of columns in the table */
int CellLibTableModel::columnCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return 3;
}

QVariant CellLibTableModel::data(const QModelIndex &index, int role) const
{
    QVariant v;

    if ((m_cellLib == nullptr) || (!index.isValid()))
        return v;

    size_t idx = index.row();
    switch(role)
    {
    case Qt::DisplayRole:
    case Qt::EditRole:
        if (idx < rowCount())
        {
            auto cell = m_cellLib->m_cells.at(idx);
            if (cell != nullptr)
            {
                switch(index.column())
                {
                case 0: // cell name
                    return QString::fromStdString(cell->m_name);
                case 1: // cell class
                    //return QString("");
                    return QString::fromStdString(ChipDB::toString(cell->m_class));
                default:
                    //return QString("");
                    return QString::fromStdString(ChipDB::toString(cell->m_subclass));
                }
            }
                
            else
                return QString("(null)");
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

QVariant CellLibTableModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    const std::array<const char *,3> headerNames= 
    {
        "Name","Class","Subclass"
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

Qt::ItemFlags CellLibTableModel::flags(const QModelIndex &index) const
{
    return Qt::ItemIsEnabled | Qt::ItemIsSelectable;
}

const ChipDB::Cell* CellLibTableModel::getCell(int row) const
{
    if (m_cellLib == nullptr)
        return nullptr;

    if (row < m_cellLib->size())
    {
        return m_cellLib->m_cells.at(row);
    }
    else
    {
        return nullptr;
    }
}

void CellLibTableModel::cellLibUpdated()
{
    //synchronizeCellIds(m_cellLib);
}
