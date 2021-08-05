#include <array>
#include <QString>
#include <QColor>
#include "cellpinmodel.h"

using namespace GUI;

// ********************************************************************************
//    CellTreeItem
// ********************************************************************************

CellTreeItem::CellTreeItem()
{
    setCell(nullptr);
}

CellTreeItem::~CellTreeItem()
{
    destroy();
}

CellTreeItem::CellTreeItem(const ChipDB::Cell *cell) : m_cell(cell)
{
    setCell(cell);
}

void CellTreeItem::destroy()
{
    for(auto child : m_children)
    {
        delete child;
    }
    m_children.clear();
}

void CellTreeItem::setCell(const ChipDB::Cell *cell)
{
    destroy();

    if (cell != nullptr)
    {
        for(auto const& pin : cell->m_pins)
        {
            addPin(&pin);
        }
    }
}

ICellTreeItem* CellTreeItem::child(int row)
{
    if ((row>=0) && (row < m_children.size()))
    {
        return m_children.at(row);
    }

    return nullptr;
}

int CellTreeItem::childCount() const
{
    return m_children.size();
}

int CellTreeItem::columnCount() const
{
    return 1;
}

QVariant CellTreeItem::data(const QModelIndex &index) const
{
    auto col = index.column();
    auto row = index.row();

    std::cout << "CellTreeItem: row: " << row << " col: " << col << "\n";
    if (col < 0 || col >= m_children.size())
    {
        return QVariant();
    }

    return m_children.at(col)->getPinName();
}

int CellTreeItem::row() const
{
    return 0;
}

ICellTreeItem *CellTreeItem::parentItem()
{
    return nullptr;
}

void CellTreeItem::addPin(const ChipDB::PinInfo *pin)
{
    m_children.push_back(new CellTreePinItem(this, pin));
}

int CellTreeItem::indexOfChild(const ICellTreeItem *childPtr) const
{
    for(int idx=0; idx<m_children.size(); idx++)
    {
        if (m_children.at(idx) == childPtr)
            return idx;
    }
    return -1;
}

// ********************************************************************************
//    CellTreePinItem
// ********************************************************************************

CellTreePinItem::CellTreePinItem(ICellTreeItem *parent, const ChipDB::PinInfo *pin) : m_parent(parent), m_pin(pin)
{
}

ICellTreeItem* CellTreePinItem::child(int row)
{
    return nullptr;
}

int CellTreePinItem::childCount() const
{
    return 1;
}

int CellTreePinItem::columnCount() const
{
    return 1;
}

QVariant CellTreePinItem::data(const QModelIndex &index) const
{
    std::cout << "CellTreePinItem: row:" << index.row() << "  col:"  << index.column() << "\n";
    if (index.column() < 0 || index.column() >= 1)
    {
        return QVariant();
    }

    return "Pin data..";
}

int CellTreePinItem::row() const
{
    return m_parent->indexOfChild(this);
}

ICellTreeItem *CellTreePinItem::parentItem()
{
    return m_parent;
}

QString CellTreePinItem::getPinName() const
{
    return QString::fromStdString(m_pin->m_name);
}

// ********************************************************************************
//    CellTreeModel
// ********************************************************************************

CellTreeModel::CellTreeModel(const ChipDB::Cell *cell)
{
    setCell(cell);
}

void CellTreeModel::setCell(const ChipDB::Cell *cell)
{
    beginResetModel();
    m_rootItem.reset(new CellTreeItem(cell));
    endResetModel();
}

int CellTreeModel::rowCount(const QModelIndex &parent) const
{
    ICellTreeItem *parentItem;
    if (parent.column() > 0)
        return 0;

    if (!parent.isValid())
    {
        parentItem = m_rootItem.get();
    }
    else
    {
        parentItem = static_cast<ICellTreeItem*>(parent.internalPointer());
    }

    return parentItem->childCount();
}

int CellTreeModel::columnCount(const QModelIndex &parent) const
{
    if (parent.isValid())
        return static_cast<ICellTreeItem*>(parent.internalPointer())->columnCount();

    return m_rootItem->columnCount();
}

QVariant CellTreeModel::data(const QModelIndex &index, int role) const
{
    std::cout << "Data row: " << index.row() << "  col: " << index.column() << "  Ptr: " << index.internalPointer() << "\n";
    std::cout << std::flush;    

    if (!index.isValid())
        return QVariant();

    if (role != Qt::DisplayRole)
        return QVariant();

    ICellTreeItem *item = static_cast<ICellTreeItem*>(index.internalPointer());

    return item->data(index);
}


QModelIndex CellTreeModel::index(int row, int column,
    const QModelIndex &parent) const
{
    std::cout << "Index row: " << row << "  col: " << column << "  parent row: " << parent.row() << "  parent col: " << parent.column() << "\n";
    std::cout << std::flush;

    if (!hasIndex(row, column, parent))
        return QModelIndex();

    ICellTreeItem *parentItem;

    if (!parent.isValid())
    {
        parentItem = static_cast<ICellTreeItem*>(m_rootItem.get());
    }
    else
    {
        parentItem = static_cast<ICellTreeItem*>(parent.internalPointer());
    }

    ICellTreeItem *childItem = parentItem->child(row);
    if (childItem)
    {
        return createIndex(row, column, childItem);
    }

    return QModelIndex();
};

QModelIndex CellTreeModel::parent(const QModelIndex &index) const
{
    std::cout << "Parent row: " << index.row() << " col:" << index.column() << "\n";

    if (!index.isValid())
        return QModelIndex();

    ICellTreeItem *childItem = static_cast<ICellTreeItem*>(index.internalPointer());
    std::cout << "  childItem ptr = " << childItem << "\n";

    ICellTreeItem *parentItem = childItem->parentItem();
    std::cout << "  parentItem ptr = " << parentItem << "\n";

    if (parentItem == static_cast<ICellTreeItem*>(m_rootItem.get()))
        return QModelIndex();

    std::cout << "  result row = " << parentItem->row() << "\n";

    return createIndex(parentItem->row(), 0, parentItem);
}

#if 0
class CellTreeModel : public QAbstractItemModel
{
public:
    CellTreeModel(const ChipDB::Cell *cell);

    void setCell(const ChipDB::Cell *cell);

    QVariant data(const QModelIndex &index, int role) const override;
    
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;

protected:
    const ChipDB::Cell *m_cell;
};
#endif


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
            case 6:
                return "Clock pin";
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
            case 6:
                return m_pinInfo->m_clock;
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
