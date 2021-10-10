
#include <array>
#include <QString>
#include <QColor>
#include <QIcon>
#include <QPainter>
#include "layermodel.h"

using namespace GUI;

LayerListModel::LayerListModel(LayerRenderInfoDB *layers) : m_layers(nullptr)
{
    m_lightColor = QColor("#F0F0F0");
    m_darkColor  = QColor("#D0D0D0");
    setLayers(layers);
}

void LayerListModel::setLayers(LayerRenderInfoDB *layers)
{
    if (m_layers != nullptr)
    {
        m_layers->removeListener(this);
    }

    beginResetModel();
    m_layers = layers;
    endResetModel();

    if (m_layers != nullptr)
    {
        m_layers->addListener(this);
    }    
}

int LayerListModel::rowCount(const QModelIndex &parent) const
{
    if (m_layers == nullptr)
        return 0;

    return m_layers->size();
}

QVariant LayerListModel::data(const QModelIndex &index, int role) const
{
    QVariant v;

    if ((m_layers == nullptr) || (!index.isValid()))
        return v;

    size_t idx = index.row();
    switch(role)
    {
    case Qt::DisplayRole:
    case Qt::EditRole:
        if (idx < rowCount())
        {
            auto layerPtr = m_layers->at(idx);
            if (layerPtr != nullptr)
                return QString::fromStdString(layerPtr->getName());
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
    case Qt::DecorationRole:
        if (idx < m_pixmapCache.size())
        {              
            return m_pixmapCache.at(idx).m_pixmap;
        }
        break;
    }

    return v;
}

QVariant LayerListModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    return "";
}

Qt::ItemFlags LayerListModel::flags(const QModelIndex &index) const
{
    return Qt::ItemIsEnabled | Qt::ItemIsSelectable;
}

void LayerListModel::updatePixmapCache()
{
    if (m_layers == nullptr)
    {
        return;
    }

    m_pixmapCache.resize(m_layers->size());

    // update icon cache
    ssize_t idx = 0;
    while(idx < m_layers->size())
    {
        QPixmap pixmap;
        auto layer = m_layers->at(idx);
        if (layer != nullptr)
        {
            pixmap = layer->routing().createToolbarPixmap(QSize(24,24), Qt::black);
        }

        m_pixmapCache.at(idx).m_pixmap = pixmap;
        idx++;
    }
}

void LayerListModel::notify(ssize_t index, NotificationType t)
{
    beginResetModel();
    endResetModel();

    updatePixmapCache();
}
