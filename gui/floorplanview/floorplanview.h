// SPDX-FileCopyrightText: 2021-2022 Niels Moseley <asicsforthemasses@gmail.com>
//
// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <QWidget>
#include "common/guihelpers.h"
#include "common/database.h"

namespace GUI
{

struct FloorplanOverlayBase
{
    virtual void paint(QPainter &painter, const Viewport &vp) = 0;
};

class FloorplanView : public QWidget
{
    Q_OBJECT

public:
    explicit FloorplanView(QWidget *parent = nullptr);
    virtual ~FloorplanView();

    QSize sizeHint() const override;

    /** set the database which contains the floorplan object */
    void setDatabase(std::shared_ptr<Database> db);

    /** set an overlay object that will draw on to of the floorplan */
    void setOverlay(FloorplanOverlayBase *overlay = nullptr);

    /** enable the mouse curor crosshair for position feedback */
    void setCrosshair(bool enabled = true) noexcept
    {
        m_crosshairEnabled = enabled;
    }

    void showNets(bool enabled = true) noexcept
    {
        m_showNets = enabled;
    }

protected:
    void mousePressEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void leaveEvent(QEvent *event) override;
    void wheelEvent(QWheelEvent *event) override;

    void paintEvent(QPaintEvent *event) override;

    void resizeEvent(QResizeEvent *event) override;

    void drawInstances(QPainter &p);
    void drawRegions(QPainter &p);
    void drawNets(QPainter &p);
    void drawRows(QPainter &p, const std::shared_ptr<ChipDB::Region> region);

    void drawCell(QPainter &p, const std::shared_ptr<ChipDB::Instance> ins);
    void drawPin(QPainter &p, const std::shared_ptr<ChipDB::Instance> ins);

    void drawBottomRuler(QPainter &p);
    void drawLeftRuler(QPainter &p);

    void drawNet(QPainter &p, const std::shared_ptr<ChipDB::Net> net);
    
    void drawGlobalRouterGrid(QPainter &p, const std::shared_ptr<LunaCore::GlobalRouter::Grid> grid);

    Viewport m_viewPort;    

    enum class MouseState
    {
        None,
        Dragging
    } m_mouseState;

    ChipDB::Rect64 m_viewPortRef;
    QPoint m_mouseDownPos;
    QPoint m_mousePos;

    FloorplanOverlayBase* m_overlay{nullptr};

    std::shared_ptr<Database> m_db;
    bool  m_dirty{true};

    bool  m_crosshairEnabled{true};
    bool  m_showNets{false};
    bool  m_showGlobalRouterGrid{false};
};

};  // namespace