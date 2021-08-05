/*

*/

#pragma once

#include <QAbstractListModel>
#include <QAbstractTableModel>
#include <string>
#include <vector>

#include "lunacore.h"

namespace GUI
{

class ICellTreeItem
{
public:
    virtual ICellTreeItem* child(int row) = 0;
    virtual int childCount() const = 0;
    virtual int columnCount() const = 0;

    virtual QVariant data(const QModelIndex &index) const = 0;
    virtual int row() const = 0;
    virtual ICellTreeItem *parentItem() = 0;

    virtual int indexOfChild(const ICellTreeItem *childPtr) const
    {
        return -1;
    }
};

class CellTreePinItem : public ICellTreeItem
{
public:
    CellTreePinItem(ICellTreeItem *parent, const ChipDB::PinInfo *pin);

    ICellTreeItem* child(int row) override;
    int childCount() const override;
    int columnCount() const override;

    QVariant data(const QModelIndex &index) const override;
    int row() const override;
    ICellTreeItem *parentItem() override;

    QString getPinName() const;

protected:
    ICellTreeItem *m_parent;
    const ChipDB::PinInfo *m_pin;
};

class CellTreeItem : public ICellTreeItem
{
public:
    CellTreeItem();
    CellTreeItem(const ChipDB::Cell *cell);

    virtual ~CellTreeItem();

    void setCell(const ChipDB::Cell *cell);

    ICellTreeItem* child(int row) override;
    int childCount() const override;
    int columnCount() const override;
    QVariant data(const QModelIndex &index) const override;
    int row() const override;
    ICellTreeItem *parentItem() override;

    int indexOfChild(const ICellTreeItem *childPtr) const override;

protected:
    void destroy();
    void addPin(const ChipDB::PinInfo *pin);
    
    std::vector<CellTreePinItem*> m_children;
    const ChipDB::Cell *m_cell;
};


class CellTreeModel : public QAbstractItemModel
{
public:
    CellTreeModel(const ChipDB::Cell *cell);

    void setCell(const ChipDB::Cell *cell);

    QVariant data(const QModelIndex &index, int role) const override;

    QModelIndex index(int row, int column,
        const QModelIndex &parent = QModelIndex()) const override;

    QModelIndex parent(const QModelIndex &index) const override;

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;

protected:
    std::unique_ptr<CellTreeItem> m_rootItem;
};


class CellPinListModel : public QAbstractListModel
{
public:
    CellPinListModel(const ChipDB::Cell *cell);

    void setCell(const ChipDB::Cell *cell);

    /** return the number of rows in the list */
    virtual int rowCount(const QModelIndex &parent = QModelIndex()) const override;

    /** returns various information about enabled/disabled items etc */
    Qt::ItemFlags flags(const QModelIndex &index) const override;

    /** query the data a 'index' position */
    virtual QVariant data(const QModelIndex &index, int role) const override;

    /** query the view/list header information */
    virtual QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

    const ChipDB::PinInfo* getPinInfo(int row);

    void cellUpdated();

protected:
    const ChipDB::Cell *m_cell;
};



class PinInfoTableModel : public QAbstractTableModel
{
public:
    PinInfoTableModel(const ChipDB::PinInfo *pinInfo);

    void setPinInfo(const ChipDB::PinInfo *pinInfo);

    /** return the number of rows in the table */
    virtual int rowCount(const QModelIndex &parent = QModelIndex()) const override;

    /** return the number of rows in the table */
    virtual int columnCount(const QModelIndex &parent = QModelIndex()) const override;

    /** returns various information about enabled/disabled items etc */
    Qt::ItemFlags flags(const QModelIndex &index) const override;

    /** query the data a 'index' position */
    virtual QVariant data(const QModelIndex &index, int role) const override;

    /** query the view/list header information */
    virtual QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

    //ChipDB::PinInfo getPinInfo(int row) const;

    void pinUpdated();

protected:
    const ChipDB::PinInfo* m_pinInfo;    
};

};
