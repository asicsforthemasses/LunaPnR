// SPDX-FileCopyrightText: 2021-2025 Niels Moseley <asicsforthemasses@gmail.com>
//
// SPDX-License-Identifier: GPL-3.0-only

#include "modulemodel.h"

using namespace GUI;

ModuleTableModel::ModuleTableModel(std::shared_ptr<ChipDB::ModuleLib> moduleLib) : m_moduleLib(nullptr)
{
    m_lightColor = QColor("#F0F0F0");
    m_darkColor  = QColor("#D0D0D0");
    setModuleLib(moduleLib);
}

void ModuleTableModel::setModuleLib(std::shared_ptr<ChipDB::ModuleLib> moduleLib)
{
    if (m_moduleLib)
    {
        m_moduleLib->removeListener(this);
    }

    beginResetModel();
    m_moduleLib = moduleLib;
    endResetModel();

    if (m_moduleLib)
    {
        m_moduleLib->addListener(this);
    }
}

void ModuleTableModel::notify(ChipDB::ObjectKey index, NotificationType t)
{
    beginResetModel();
    endResetModel();
}

int ModuleTableModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    if (!m_moduleLib)
        return 0;

    return m_moduleLib->size();
}

/** return the number of columns in the table */
int ModuleTableModel::columnCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return 1;
}

QVariant ModuleTableModel::data(const QModelIndex &index, int role) const
{
    QVariant v;

    if ((!m_moduleLib) || (!index.isValid()))
        return v;

    // FIXME: we need to use keys here
    //        because there might be gaps in the numbering
    //        easiest way is to use the user data for each item
    size_t idx = index.row();
    switch(role)
    {
    case Qt::DisplayRole:
    case Qt::EditRole:
        if (idx < rowCount())
        {
            auto module = m_moduleLib->lookupModule(idx);
            if (module)
            {
                switch(index.column())
                {
                case 0: // cell name
                    return QString::fromStdString(module->name());
                //case 1: // cell class
                    //return QString("");
                    //return QString::fromStdString(ChipDB::toString(cell->m_class));
                default:
                    break;
                    //return QString("");
                    //return QString::fromStdString(ChipDB::toString(cell->m_subclass));
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

QVariant ModuleTableModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    const std::array<const char *,1> headerNames=
    {
        "Module"
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

Qt::ItemFlags ModuleTableModel::flags(const QModelIndex &index) const
{
    return Qt::ItemIsEnabled | Qt::ItemIsSelectable;
}

const std::shared_ptr<ChipDB::Module> ModuleTableModel::getModule(int row) const
{
    if (!m_moduleLib)
        return nullptr;

    if (row < m_moduleLib->size())
    {
        return m_moduleLib->lookupModule(row);
    }
    else
    {
        return nullptr;
    }
}


// ********************************************************************************
//    ModuleListModel
// ********************************************************************************

ModuleListModel::ModuleListModel(std::shared_ptr<ChipDB::ModuleLib> moduleLib) : m_moduleLib(nullptr)
{
    setModuleLib(moduleLib);
}

int ModuleListModel::rowCount(const QModelIndex &parent) const
{
    if (m_moduleLib == nullptr)
    {
        return 0;
    }
    return m_moduleLib->size();
}

void ModuleListModel::setModuleLib(std::shared_ptr<ChipDB::ModuleLib> moduleLib)
{
    if (m_moduleLib)
    {
        m_moduleLib->removeListener(this);
    }

    beginResetModel();
    m_moduleLib = moduleLib;
    endResetModel();

    if (m_moduleLib)
    {
        m_moduleLib->addListener(this);
    }
}

    /** query the view/list header information */
QVariant ModuleListModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    return "Modules";
}

Qt::ItemFlags ModuleListModel::flags(const QModelIndex &index) const
{
    return Qt::ItemIsEnabled;
}

QVariant ModuleListModel::data(const QModelIndex &index, int role) const
{
    QVariant v;

    if ((m_moduleLib == nullptr) || (!index.isValid()))
        return v;

    size_t idx = index.row();
    switch(role)
    {
    case Qt::DisplayRole:
    case Qt::EditRole:
        if (idx < rowCount())
        {
            auto module = m_moduleLib->lookupModule(idx);
            if (module != nullptr)
            {
                return QString::fromStdString(module->name());
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

void ModuleListModel::notify(ChipDB::ObjectKey index, NotificationType t)
{
    beginResetModel();
    endResetModel();
}
