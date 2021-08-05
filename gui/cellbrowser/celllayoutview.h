
#pragma once

#include <QWidget>
#include <QPainter>
#include "lunacore.h"


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

protected:
    void paintEvent(QPaintEvent *event) override;

protected:
    /** make sure p1 and p2 are upper left and lower right */
    void fixCoordinates(QPointF &p1, QPointF &p2);

    QPointF toScreen(const ChipDB::Coord64 &pos) const;
    const ChipDB::Coord64 toChip(const QPointF &p) const;

    
    const ChipDB::Cell *m_cell;
};

};  // namespace