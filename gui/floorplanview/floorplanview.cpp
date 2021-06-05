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

void FloorplanView::drawInstance(QPainter &p, const ChipDB::InstanceBase *ins)
{
    if (ins != nullptr)
    {
        QRectF cellRect;
        cellRect.setBottomLeft(toScreen(ins->m_pos, width(), height()));
        cellRect.setTopRight(toScreen(ins->m_pos + ins->instanceSize(), width(), height()));
        p.drawRect(cellRect);

        switch(ins->m_orientation)
        {
        case ChipDB::ORIENT_R0: // aka North
            {
                auto p1 = cellRect.bottomLeft() - QPointF{0, cellRect.height() / 2};
                auto p2 = cellRect.bottomLeft() + QPointF{cellRect.width() / 2, 0};
                p.drawLine(p1, p2);
            }
            break;
        case ChipDB::ORIENT_MX: // aka flipped South
            {
                auto p1 = cellRect.topLeft() + QPointF{0, cellRect.height() / 2};
                auto p2 = cellRect.topLeft() + QPointF{cellRect.width() / 2, 0};
                p.drawLine(p1, p2);
            }            
            break;
        default:
            break;            
        }

        // check if there is enough room to display the cell type
        // if so, draw the instance name and archetype
        QFontMetrics fm(font());
        int textWidth = fm.horizontalAdvance(QString::fromStdString(ins->getArchetypeName()));

        if (cellRect.width() > textWidth)
        {
            drawCenteredText(p, cellRect.center(), ins->getArchetypeName(), font());
        }

        textWidth = fm.horizontalAdvance(QString::fromStdString(ins->m_name));

        if (cellRect.width() > textWidth)
        {
            auto txtpoint = cellRect.center();
            txtpoint += {0, static_cast<qreal>(fm.height())};
            drawCenteredText(p, txtpoint, ins->m_name, font());
        }


    }
}
