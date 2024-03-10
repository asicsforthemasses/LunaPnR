// SPDX-FileCopyrightText: 2021-2024 Niels Moseley <asicsforthemasses@gmail.com>
//
// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <memory>
#include "lunacore.h"
#include "modelnodebase.h"
#include "../common/guihelpers.h"

namespace GUI
{

class SiteTableModel : public QAbstractTableModel, ChipDB::INamedStorageListener
{
public:
    SiteTableModel(std::shared_ptr<ChipDB::TechLib> techLib);

    virtual ~SiteTableModel();

    void setTechLib(std::shared_ptr<ChipDB::TechLib> m_techLib);

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

    const std::shared_ptr<ChipDB::SiteInfo> getSite(int row) const;

    void notify(ChipDB::ObjectKey index, ChipDB::INamedStorageListener::NotificationType t) override;

protected:
    QColor m_darkColor;
    QColor m_lightColor;
    std::shared_ptr<ChipDB::TechLib> m_techLib;
};


#if 0
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

    void setTechLib(std::shared_ptr<ChipDB::TechLib> techlib);

};
#endif

class SiteInfoNode : public NodeBase
{
public:
    SiteInfoNode(const QString &valueName, const QVariant &value,
        const QColor &bkColor);
};


class SiteInfoModel : public ModelBase, public ChipDB::INamedStorageListener
{
public:
    SiteInfoModel();
    virtual ~SiteInfoModel();

    void setSite(std::shared_ptr<ChipDB::SiteInfo> site);

    void notify(ChipDB::ObjectKey index, ChipDB::INamedStorageListener::NotificationType t) override;

protected:
    AlternatingColors m_altColors;
};

};  // namespace