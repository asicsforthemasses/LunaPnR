#include "floorplanview.h"
#include "common/guihelpers.h"

#include <QPainter>

using namespace GUI;

FloorplanView::FloorplanView(QWidget *parent) : QWidget(parent)
{
    m_netlist = nullptr;
    m_dirty   = true;
}

FloorplanView::~FloorplanView()
{

}

QSize FloorplanView::sizeHint() const
{
    return QWidget::sizeHint();    
}

void FloorplanView::setFloorplan(const ChipDB::Netlist *nl)
{
    m_netlist = nl;
    m_dirty = true;
}

void FloorplanView::mousePressEvent(QMouseEvent *event)
{
}

void FloorplanView::mouseReleaseEvent(QMouseEvent *event)
{
}

void FloorplanView::mouseMoveEvent(QMouseEvent *event)
{
}

void FloorplanView::wheelEvent(QWheelEvent *event)
{
}

void FloorplanView::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);

    m_dirty = false;

    // draw background
    painter.fillRect(rect(), Qt::black);

    if (m_netlist == nullptr)
        return;

    // TODO: draw ruler

    // TODO: draw cells
    painter.setBrush(Qt::NoBrush);
    painter.setPen(Qt::darkGray);
    for(auto const ins : m_netlist->m_instances)
    {
        drawInstance(painter, ins);
    }
}

constexpr const QPointF toScreen(const ChipDB::Coord64 &p, double sw, double sh)
{
    // going from floorplan coordinate system to 
    // widget screen space

    const double cw = 100000.0; // floorplan view width 
    const double ch = 100000.0; // floorplan view height
    const double ox = 0.0;      // floorplan view origin x
    const double oy = 0.0;      // floorplan view origin y

    double sx = (p.m_x - ox) / cw * sw;
    double sy = sh - 1 - (p.m_y - oy) / ch * sh;

    return QPointF{sx, sy};
}

void FloorplanView::drawInstance(QPainter &p, const ChipDB::Instance *ins)
{
    if (ins != nullptr)
    {
        QRectF cellRect;
        cellRect.setBottomLeft(toScreen(ins->m_pos, width(), height()));
        cellRect.setTopRight(toScreen(ins->m_pos + ins->cellSize(), width(), height()));
        p.drawRect(cellRect);

        // check if there is enough room to display text
        // if so, draw the instance name and archetype
        QFontMetrics fm(font());
        int textWidth = fm.horizontalAdvance(QString::fromStdString(ins->getArchetypeName()));

        if (cellRect.width() > textWidth)
        {
            drawCenteredText(p, cellRect.center(), ins->getArchetypeName(), font());
        }
    }
}
