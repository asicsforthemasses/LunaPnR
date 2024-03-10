// SPDX-FileCopyrightText: 2021-2024 Niels Moseley <asicsforthemasses@gmail.com>
//
// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include "common/layerrenderinfo.h"
#include <QColor>
#include <QAbstractTableModel>

namespace GUI
{

class LayerAppearanceTableModel : public QAbstractTableModel, public ChipDB::INamedStorageListener
{
public:
    LayerAppearanceTableModel(LayerRenderInfoDB *layers = nullptr);

    void setLayers(LayerRenderInfoDB *layers);

    /** return the number of rows in the list */
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;

    /** return the number of columns in the list */
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;

    /** returns various information about enabled/disabled items etc */
    Qt::ItemFlags flags(const QModelIndex &index) const override;

    /** query the data a 'index' position */
    QVariant data(const QModelIndex &index, int role) const override;

    /** query the view/list header information */
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

    void notify(ChipDB::ObjectKey index, NotificationType t) override;

protected:
    void updatePixmapCache();

    QColor m_lightColor;
    QColor m_darkColor;

    LayerRenderInfoDB *m_layers;

    struct PixmapCacheEntry
    {
        QPixmap     m_pixmap;
    };

    std::vector<PixmapCacheEntry> m_pixmapCache;
    std::vector<PixmapCacheEntry> m_pixmapObsCache;
};

};
