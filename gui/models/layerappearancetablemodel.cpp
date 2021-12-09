
#include <array>
#include <QString>
#include <QColor>
#include <QIcon>
#include <QPainter>
#include "layerappearancetablemodel.h"

using namespace GUI;

LayerAppearanceTableModel::LayerAppearanceTableModel(LayerRenderInfoDB *layers) : m_layers(nullptr)
{
    m_lightColor = QColor("#F0F0F0");
    m_darkColor  = QColor("#D0D0D0");
    setLayers(layers);
}

void LayerAppearanceTableModel::setLayers(LayerRenderInfoDB *layers)
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

int LayerAppearanceTableModel::rowCount(const QModelIndex &parent) const
{
    if (m_layers == nullptr)
        return 0;

    return m_layers->size();
}

int LayerAppearanceTableModel::columnCount(const QModelIndex &parent) const
{
    return 2;
}

QVariant LayerAppearanceTableModel::data(const QModelIndex &index, int role) const
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
            {
                if (index.column() == 0)
                {
                    return QString::fromStdString(layerPtr->getName());
                }
                else
                {
                    return QString::fromStdString("obstruct");
                }
            }
            else
            {
                return QString("(null)");
            }
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
            if (index.column() == 0)
            {
                return m_pixmapCache.at(idx).m_pixmap;
            }
            else
            {
                return m_pixmapObsCache.at(idx).m_pixmap;
            }
        }
        break;
    }

    return v;
}

QVariant LayerAppearanceTableModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    return "";
}

Qt::ItemFlags LayerAppearanceTableModel::flags(const QModelIndex &index) const
{
    return Qt::ItemIsEnabled | Qt::ItemIsSelectable;
}

void LayerAppearanceTableModel::updatePixmapCache()
{
    if (m_layers == nullptr)
    {
        return;
    }

    m_pixmapCache.resize(m_layers->size());
    m_pixmapObsCache.resize(m_layers->size());

    // update icon cache
    ssize_t idx = 0;
    while(idx < m_layers->size())
    {
        QPixmap pixmap;
        QPixmap obsPixmap;
        auto layer = m_layers->at(idx);
        if (layer != nullptr)
        {
            pixmap = layer->routing().createToolbarPixmap(QSize(24,24), Qt::black);
            obsPixmap = layer->obstruction().createToolbarPixmap(QSize(24,24), Qt::black);
        }

        m_pixmapCache.at(idx).m_pixmap = pixmap;
        m_pixmapObsCache.at(idx).m_pixmap = obsPixmap;
        idx++;
    }
}

void LayerAppearanceTableModel::notify(int32_t userID, ssize_t index, NotificationType t)
{
    beginResetModel();
    endResetModel();

    updatePixmapCache();
}
