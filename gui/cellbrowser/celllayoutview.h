
#pragma once

#include <QWidget>
#include <QPainter>
#include "lunacore.h"
#include "../common/layerrenderinfo.h"

namespace GUI
{

class CellLayoutView;   // pre-declaration

class DrawObstructionVisitor : public ChipDB::ConstVisitor
{
public:
    DrawObstructionVisitor(CellLayoutView *view, QPainter *painter);

    //virtual void visit(const ChipDB::CellRect *rect) override;

    //virtual void visit(const ChipDB::CellPolygon *rect) override;
    
protected:
    QPainter       *m_painter;
    CellLayoutView *m_view;
};

class DrawLayoutItemVisitor : public ChipDB::ConstVisitor
{
public:
    DrawLayoutItemVisitor(CellLayoutView *view, QPainter *painter);

    //virtual void visit(const ChipDB::CellRect *rect) override;

    //virtual void visit(const ChipDB::CellPolygon *rect) override;
    
    QRectF getTextRect() const;

protected:
    QPainter       *m_painter;
    CellLayoutView *m_view;
    int64_t        m_largestArea;
    QRectF         m_txtrect;
};

/** display standard cell layout */
class CellLayoutView : public QWidget
{
    Q_OBJECT

    //QSize minimumSizeHint() const override;
    //QSize sizeHint() const override;

public:    
    friend DrawObstructionVisitor;
    friend DrawLayoutItemVisitor;

    explicit CellLayoutView(QWidget *parent = nullptr);
    virtual ~CellLayoutView();

    void setCell(const ChipDB::Cell *cell);
    void setLayerRenderInfoDB(LayerRenderInfoDB *renderInfoDB);

protected:
    void paintEvent(QPaintEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void wheelEvent(QWheelEvent *event) override;

    enum class MouseState
    {
        None,
        Dragging
    } m_mouseState;
    
    QPoint m_mouseDownPos;

    /** make sure p1 and p2 are upper left and lower right */
    void fixCoordinates(QPointF &p1, QPointF &p2);

    QPointF toScreen(const ChipDB::Coord64 &pos) const noexcept;
    ChipDB::Coord64 toChip(const QPointF &p) const noexcept;
    ChipDB::Coord64 toChipDelta(const QPointF &delta) const noexcept;

    void drawGeometry(QPainter &painter, const ChipDB::GeometryObjects &objs, const QBrush &brush) const;
    void drawGeometry(QPainter &painter, const ChipDB::Rectangle &objs) const;
    void drawGeometry(QPainter &painter, const ChipDB::Polygon &objs) const;

    std::optional<LayerRenderInfo> getLayerRenderInfo(ChipDB::LayerID id) const;

    ChipDB::Rect64 m_viewportStartDrag;
    ChipDB::Rect64 m_viewport;
    int32_t        m_zoomLevel;

    const ChipDB::Cell      *m_cell;
    const LayerRenderInfoDB *m_renderInfoDB;
};

};  // namespace