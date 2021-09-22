
#include "modulemodel.h"

using namespace GUI;

ModuleTableModel::ModuleTableModel(const ChipDB::ModuleLib *moduleLib)
{
    m_lightColor = QColor("#F0F0F0");
    m_darkColor  = QColor("#D0D0D0");  
    setModuleLib(moduleLib);
}

void ModuleTableModel::setModuleLib(const ChipDB::ModuleLib *moduleLib)
{
    beginResetModel();
    m_moduleLib = moduleLib;
    endResetModel();
}

int ModuleTableModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    if (m_moduleLib == nullptr)
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

    if ((m_moduleLib == nullptr) || (!index.isValid()))
        return v;

    size_t idx = index.row();
    switch(role)
    {
    case Qt::DisplayRole:
    case Qt::EditRole:
        if (idx < rowCount())
        {
            auto module = m_moduleLib->m_modules.at(idx);
            if (module != nullptr)
            {
                switch(index.column())
                {
                case 0: // cell name
                    return QString::fromStdString(module->m_name);
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
    case Qt::BackgroundColorRole:
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

const ChipDB::Module* ModuleTableModel::getModule(int row) const
{
    if (m_moduleLib == nullptr)
        return nullptr;

    if (row < m_moduleLib->size())
    {
        return m_moduleLib->m_modules.at(row);
    }
    else
    {
        return nullptr;
    }
}


// ********************************************************************************
//    ModuleListModel
// ********************************************************************************

ModuleListModel::ModuleListModel(const ChipDB::ModuleLib *moduleLib)
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

void ModuleListModel::setModuleLib(const ChipDB::ModuleLib *moduleLib)
{
    beginResetModel();
    m_moduleLib = moduleLib;
    endResetModel();
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
            auto module = m_moduleLib->m_modules.at(idx);
            if (module != nullptr)
            {
                return QString::fromStdString(module->m_name);
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
