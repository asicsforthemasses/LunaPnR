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

class CellPinListModel : public QAbstractListModel
{
public:
    CellPinListModel(const ChipDB::Cell *m_cell);

    void setCell(const ChipDB::Cell *m_cell);

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
