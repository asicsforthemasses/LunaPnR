#include "floorplanview.h"
#include "common/guihelpers.h"

#include <QPainter>
#include <QWheelEvent>
#include <QMouseEvent>

using namespace GUI;

constexpr const QPointF toScreen(const ChipDB::Coord64 &p, const QRectF &viewport,
    const QSizeF &screenSize)
{
    // going from floorplan coordinate system to 
    // widget screen space

    //const double cw = 100000.0; // floorplan view width 
    //const double ch = 100000.0; // floorplan view height
    //const double ox = 0.0;      // floorplan view origin x
    //const double oy = 0.0;      // floorplan view origin y

    const double sx = (p.m_x - viewport.left()) / viewport.width() * screenSize.width();
    const double sy = screenSize.height() - 1.0 - (p.m_y - viewport.top()) / viewport.height() * screenSize.height();

    return QPointF{sx, sy};
}

constexpr const ChipDB::Coord64 fromScreen(const QPointF &p, const QRectF &viewport,
    const QSizeF &screenSize)
{
    const double x = (p.x() * viewport.width()) / screenSize.width() + viewport.left();
    const double y = viewport.top() - (p.y()-screenSize.height() + 1.0)/screenSize.height() * viewport.height();
    return ChipDB::Coord64{static_cast<int64_t>(x),static_cast<int64_t>(y)};
}

FloorplanView::FloorplanView(QWidget *parent) : QWidget(parent)
{
    m_viewPort.setHeight(100000.0);
    m_viewPort.setWidth(100000.0);
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
    QPoint numPixels = event->pixelDelta();
    QPoint numDegrees = event->angleDelta() / 8;

    //FIXME: for 5.14 it is called position()
    auto mousePos = event->pos();
    auto mousePosInViewport = fromScreen(mousePos, m_viewPort, size());

    auto tmpViewPort = m_viewPort;
    
    if (!numPixels.isNull()) 
    {
        tmpViewPort.translate(-mousePosInViewport.m_x, -mousePosInViewport.m_y);
        if (numPixels.ry() > 0)
        {
            tmpViewPort.setHeight(tmpViewPort.height() * 1.1);
            tmpViewPort.setWidth(tmpViewPort.width() * 1.1);
            tmpViewPort.setX(tmpViewPort.x() * 1.1);
            tmpViewPort.setY(tmpViewPort.y() * 1.1);
            tmpViewPort.translate(mousePosInViewport.m_x, mousePosInViewport.m_y);
            m_viewPort = tmpViewPort;
            update();
        }
        else
        {
            tmpViewPort.setHeight(tmpViewPort.height() / 1.1);
            tmpViewPort.setWidth(tmpViewPort.width() / 1.1);
            tmpViewPort.setX(tmpViewPort.x() / 1.1);
            tmpViewPort.setY(tmpViewPort.y() / 1.1);            
            tmpViewPort.translate(mousePosInViewport.m_x, mousePosInViewport.m_y);
            m_viewPort = tmpViewPort;
            update();
        }
    } 
    else if (!numDegrees.isNull()) 
    {
        tmpViewPort.translate(-mousePosInViewport.m_x, -mousePosInViewport.m_y);
        QPoint numSteps = numDegrees / 15;
        if (numSteps.ry() > 0)
        {
            tmpViewPort.setHeight(tmpViewPort.height() * 1.1);
            tmpViewPort.setWidth(tmpViewPort.width() * 1.1);
            tmpViewPort.setX(tmpViewPort.x() * 1.1);
            tmpViewPort.setY(tmpViewPort.y() * 1.1);                        
            tmpViewPort.translate(mousePosInViewport.m_x, mousePosInViewport.m_y);
            m_viewPort = tmpViewPort;            
            update();
        }
        else
        {
            tmpViewPort.setHeight(tmpViewPort.height() / 1.1);
            tmpViewPort.setWidth(tmpViewPort.width() / 1.1);
            tmpViewPort.setX(tmpViewPort.x() / 1.1);
            tmpViewPort.setY(tmpViewPort.y() / 1.1);
            tmpViewPort.translate(mousePosInViewport.m_x, mousePosInViewport.m_y);
            m_viewPort = tmpViewPort;            
            update();
        }
    }

    event->accept();    
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

void FloorplanView::drawInstance(QPainter &p, const ChipDB::InstanceBase *ins)
{
    auto screenSize = size();
    if (ins != nullptr)
    {
        QRectF cellRect;
        cellRect.setBottomLeft(toScreen(ins->m_pos, m_viewPort, screenSize));
        cellRect.setTopRight(toScreen(ins->m_pos + ins->instanceSize(), m_viewPort, screenSize));

        // check if the instance is in view
        //if (!cellRect.intersects(m_viewPort))
        //    return;

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
