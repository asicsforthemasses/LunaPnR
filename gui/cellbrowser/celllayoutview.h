// SPDX-FileCopyrightText: 2021-2023 Niels Moseley <asicsforthemasses@gmail.com>
//
// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <QWidget>
#include <QPainter>
#include "lunacore.h"
#include "../common/database.h"

namespace GUI
{

/** display standard cell layout */
class CellLayoutView : public QWidget
{
    Q_OBJECT

public:    
    explicit CellLayoutView(QWidget *parent = nullptr);
    virtual ~CellLayoutView();

    void setCell(const std::shared_ptr<ChipDB::Cell> cell);
    void setDatabase(std::shared_ptr<Database> db);

    void showCrosshair(bool enabled = true);

protected:
    void paintEvent(QPaintEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void wheelEvent(QWheelEvent *event) override;
    void leaveEvent(QEvent *event) override;

    enum class MouseState
    {
        None,
        Dragging
    } m_mouseState;

    QPoint m_mousePos;
    QPoint m_mouseDownPos;

    /** make sure p1 and p2 are upper left and lower right */
    void fixCoordinates(QPointF &p1, QPointF &p2);

    QPointF toScreen(const ChipDB::Coord64 &pos) const noexcept;
    ChipDB::Coord64 toChip(const QPointF &p) const noexcept;
    ChipDB::Coord64 toChipDelta(const QPointF &delta) const noexcept;

    void drawGeometry(QPainter &painter, const ChipDB::GeometryObjects &objs, const QBrush &brush) const;
    void drawGeometry(QPainter &painter, const ChipDB::Rectangle &objs) const;
    void drawGeometry(QPainter &painter, const ChipDB::Polygon &objs) const;

    bool drawRouting(QPainter &painter, const std::string &layerName) const;

    ChipDB::KeyObjPair<GUI::LayerRenderInfo> getLayerRenderInfo(const std::string &layerName) const;

    ChipDB::Rect64 m_viewportStartDrag;
    ChipDB::Rect64 m_viewport;
    int32_t        m_zoomLevel;

    bool m_crosshairEnabled{true};

    ChipDB::Cell   m_cell;  ///< (partial) local copy of cell
    std::shared_ptr<Database> m_db;
};

};  // namespace