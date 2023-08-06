// SPDX-FileCopyrightText: 2021-2023 Niels Moseley <asicsforthemasses@gmail.com>
//
// SPDX-License-Identifier: GPL-3.0-only

#include <array>
#include <QString>
#include <QColor>
#include "celllibmodel.h"

using namespace GUI;

// ********************************************************************************
//    CellLibListModel
// ********************************************************************************

CellLibListModel::CellLibListModel(std::shared_ptr<ChipDB::CellLib> cellLib)
{
    m_lightColor = QColor("#F0F0F0");
    m_darkColor  = QColor("#D0D0D0");
    setCellLib(cellLib);    
}

void CellLibListModel::setCellLib(std::shared_ptr<ChipDB::CellLib> cellLib)
{
    beginResetModel();
    m_cellLib = cellLib;
    endResetModel();
}

int CellLibListModel::rowCount(const QModelIndex &parent) const
{
    if (!m_cellLib)
        return 0;

    return m_cellLib->size();
}

QVariant CellLibListModel::data(const QModelIndex &index, int role) const
{
    QVariant v;

    if ((!m_cellLib) || (!index.isValid()))
        return v;

    size_t idx = index.row();
    switch(role)
    {
    case Qt::DisplayRole:
    case Qt::EditRole:
        if (idx < rowCount())
        {
            //FIXME: use keys
            auto cell = m_cellLib->lookupCell(idx);
            if (cell != nullptr)
                return QString::fromStdString(cell->name());
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

QVariant CellLibListModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    return "";
}

Qt::ItemFlags CellLibListModel::flags(const QModelIndex &index) const
{
    return Qt::ItemIsEnabled | Qt::ItemIsSelectable;
}

std::shared_ptr<ChipDB::Cell> CellLibListModel::getCell(int row) const
{
    if (m_cellLib == nullptr)
        return nullptr;

    //FIXME: use keys
    if (row < m_cellLib->size())
    {
        return m_cellLib->lookupCell(row);
    }
    else
    {
        return nullptr;
    }
}


// ********************************************************************************
//    CellLibTableModel
// ********************************************************************************

CellLibTableModel::CellLibTableModel(std::shared_ptr<ChipDB::CellLib> cellLib) : m_cellLib(nullptr)
{
    m_lightColor = QColor("#F0F0F0");
    m_darkColor  = QColor("#D0D0D0");  
    setCellLib(cellLib);
}

CellLibTableModel::~CellLibTableModel()
{
}

void CellLibTableModel::setCellLib(std::shared_ptr<ChipDB::CellLib> cellLib)
{
    if (m_cellLib)
    {
        m_cellLib->removeListener(this);
    }

    beginResetModel();
    m_cellLib = cellLib;
    if (m_cellLib)
    {
        m_cellLib->addListener(this);
    }
    endResetModel();
}

int CellLibTableModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    if (!m_cellLib)
        return 0;

    return m_cellLib->size();
}

/** return the number of columns in the table */
int CellLibTableModel::columnCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return 3;
}

bool CellLibTableModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (index.column() != c_subclassColumn)
    {
        return false;
    }
    
    if (m_cellLib == nullptr)
    {
        return false;
    }

    if (index.row() >= m_cellLib->size())
    {
        return false;
    }

    if ((role == Qt::UserRole) && (index.column() == 2))
    {
        //FIXME: use keys
        auto subclassString = value.toString();
        m_cellLib->lookupCell(index.row())->m_subclass = subclassString.toStdString();
    }

    return true;
}

QVariant CellLibTableModel::data(const QModelIndex &index, int role) const
{
    QVariant v;

    if ((!m_cellLib) || (!index.isValid()))
        return v;

    size_t idx = index.row();
    switch(role)
    {
    case Qt::DisplayRole:
    case Qt::EditRole:
        if (idx < rowCount())
        {
            // FIXME: use keys
            auto cell = m_cellLib->lookupCell(idx);
            if (cell != nullptr)
            {
                switch(index.column())
                {
                case 0: // cell name
                    return QString::fromStdString(cell->name());
                case 1: // cell class
                    return QString::fromStdString(ChipDB::toString(cell->m_class));
                default:
                    return QString::fromStdString(ChipDB::toString(cell->m_subclass));
                }
            }
                
            else
                return QString("(null)");
        }
        break;
    case Qt::UserRole:
        if (idx < rowCount())
        {
            //FIXME: use keys
            auto cell = m_cellLib->lookupCell(idx);
            if (cell != nullptr)
            {
                switch(index.column())
                {
                case 0: // cell name
                    return 0;
                case 1: // cell class
                    return QString::fromStdString(ChipDB::toString(cell->m_class));
                default:
                    return QString::fromStdString(ChipDB::toString(cell->m_subclass));
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
    // only cell class is editable
    if (index.column() != c_subclassColumn)
    {
        return Qt::ItemIsEnabled | Qt::ItemIsSelectable;
    }
    else
    {
        return Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsEditable;
    }
}

std::shared_ptr<ChipDB::Cell> CellLibTableModel::getCell(int row) const
{
    if (m_cellLib == nullptr)
        return nullptr;

    //FIXME: use keys
    if (row < m_cellLib->size())
    {
        return m_cellLib->lookupCell(row);
    }
    else
    {
        return nullptr;
    }
}

void CellLibTableModel::notify(ChipDB::ObjectKey index, ChipDB::INamedStorageListener::NotificationType t)
{
    beginResetModel();
    endResetModel();    
}
