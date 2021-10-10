#pragma once

#include "common/layerrenderinfo.h"
#include <QColor>
#include <QAbstractListModel>

namespace GUI
{

class LayerListModel : public QAbstractListModel, public ChipDB::INamedStorageListener
{
public:
    LayerListModel(LayerRenderInfoDB *layers = nullptr);

    void setLayers(LayerRenderInfoDB *layers);

    /** return the number of rows in the list */
    virtual int rowCount(const QModelIndex &parent = QModelIndex()) const override;

    /** returns various information about enabled/disabled items etc */
    Qt::ItemFlags flags(const QModelIndex &index) const override;

    /** query the data a 'index' position */
    virtual QVariant data(const QModelIndex &index, int role) const override;

    /** query the view/list header information */
    virtual QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

    void notify(ssize_t index, NotificationType t) override;

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
};

};
