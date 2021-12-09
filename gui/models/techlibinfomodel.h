#pragma once

#include <memory>
#include "lunacore.h"
#include "modelnodebase.h"
#include "../common/guihelpers.h"

namespace GUI
{

class LayerTableModel : public QAbstractTableModel, ChipDB::INamedStorageListener
{
public:
    LayerTableModel(ChipDB::TechLib *techLib);

    virtual ~LayerTableModel();

    void setTechLib(ChipDB::TechLib *m_techLib);

    /** return the number of rows in the table */
    virtual int rowCount(const QModelIndex &parent = QModelIndex()) const override;

    /** return the number of columns in the table */
    virtual int columnCount(const QModelIndex &parent = QModelIndex()) const override;

    /** returns various information about enabled/disabled items etc */
    Qt::ItemFlags flags(const QModelIndex &index) const override;

    /** query the data a 'index' position */
    virtual QVariant data(const QModelIndex &index, int role) const override;

    /** query the view/list header information */
    virtual QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

    const ChipDB::LayerInfo* getLayer(int row) const;

    void notify(int32_t userID, ssize_t index, ChipDB::INamedStorageListener::NotificationType t) override;

protected:
    QColor m_darkColor;
    QColor m_lightColor;
    ChipDB::TechLib *m_techLib;
};


class TechLibInfoNode : public NodeBase
{
public:
    TechLibInfoNode(const QString &valueName, const QVariant &value);
};

class TechLibInfoModel : public ModelBase
{
public:    
    TechLibInfoModel();
    virtual ~TechLibInfoModel();

    void setTechLib(const ChipDB::TechLib *techlib);
    
};


class LayerInfoNode : public NodeBase
{
public:
    LayerInfoNode(const QString &valueName, const QVariant &value,
        const QColor &bkColor);
};


class LayerInfoModel : public ModelBase
{
public:
    LayerInfoModel();
    virtual ~LayerInfoModel();

    void setLayer(const ChipDB::LayerInfo *layer);

protected:
    AlternatingColors m_altColors;
};

};  // namespace