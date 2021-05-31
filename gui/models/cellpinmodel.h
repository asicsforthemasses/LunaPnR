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

    ChipDB::PinInfo getPinInfo(int row) const;

    void cellUpdated();

protected:
    const ChipDB::Cell *m_cell;
};

};
