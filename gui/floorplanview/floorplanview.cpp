#include "floorplanview.h"
#include "common/guihelpers.h"

#include <QPainter>
#include <QWheelEvent>
#include <QMouseEvent>

using namespace GUI;

#if 0
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

constexpr const QRectF toScreen(const ChipDB::Rect64 &p, const QRectF &viewport,
    const QSizeF &screenSize)
{
    auto ll = toScreen(p.m_ll, viewport, screenSize);
    auto ur = toScreen(p.m_ur, viewport, screenSize);

    return QRectF(ll,ur);
}

constexpr const ChipDB::Coord64 fromScreen(const QPointF &p, const QRectF &viewport,
    const QSizeF &screenSize)
{
    const double x = (p.x() * viewport.width()) / screenSize.width() + viewport.left();
    const double y = viewport.top() - (p.y()-screenSize.height() + 1.0)/screenSize.height() * viewport.height();
    return ChipDB::Coord64{static_cast<int64_t>(x),static_cast<int64_t>(y)};
}

constexpr const ChipDB::Coord64 deltaFromScreen(const QPointF &screenp1, const QPointF &screenp2,
    const QRectF &viewport, const QSizeF &screenSize)
{
    auto screenDelta = screenp2 - screenp1;
    const double x = screenDelta.x() * (viewport.width() / static_cast<double>(screenSize.width()));
    const double y = -screenDelta.y() * (viewport.height() / static_cast<double>(screenSize.height()));
    return ChipDB::Coord64{static_cast<int64_t>(x), static_cast<int64_t>(y)};
}
#endif

FloorplanView::FloorplanView(QWidget *parent) : QWidget(parent), m_db(nullptr), 
    m_mouseState(MouseState::None),
    m_overlay(nullptr)
{
    // make sure viewport pixels are 1:1

    if (width() > height())
    {
        auto w = static_cast<int64_t>(100000.0 / static_cast<double>(height()) * width());
        m_viewPort.setViewportRect(ChipDB::Rect64({0,0}, {w, 100000}));
    }
    else
    {
        auto h = static_cast<int64_t>(100000.0 / static_cast<double>(width()) * height());
        m_viewPort.setViewportRect(ChipDB::Rect64({0,0},{100000,h}));
    }

    m_viewPort.setScreenRect(rect());

    m_dirty   = true;
}

FloorplanView::~FloorplanView()
{

}

QSize FloorplanView::sizeHint() const
{
    return QWidget::sizeHint();    
}

void FloorplanView::resizeEvent(QResizeEvent *event)
{
    m_viewPort.setScreenRect(rect());

#if 0    
    if (width() > height())
    {
        m_viewPort.setScreenRect(rect());
        //m_viewPort.setWidth(m_viewPort.height() / static_cast<double>(height()) * width());
    }
    else
    {
        m_viewPort.setHeight(m_viewPortRef.width() / static_cast<double>(width()) * height());
    }
#endif
}

void FloorplanView::setDatabase(Database *db)
{
    m_db = db;
    m_dirty = true;
}

void FloorplanView::mousePressEvent(QMouseEvent *event)
{
    m_mouseDownPos = event->pos();
    m_mouseState = MouseState::Dragging;
    m_viewPortRef = m_viewPort.getViewportRect();

    setCursor(Qt::ClosedHandCursor);
}

void FloorplanView::mouseReleaseEvent(QMouseEvent *event)
{
    if (m_mouseState == MouseState::Dragging)
    {
            auto deltaInChipCoordinates = m_viewPort.toViewport(event->pos()) 
                - m_viewPort.toViewport(m_mouseDownPos);

            m_viewPort.setViewportRect(m_viewPortRef - deltaInChipCoordinates);

            setCursor(Qt::ArrowCursor);
            update();
    }
    m_mouseState = MouseState::None;
}

void FloorplanView::mouseMoveEvent(QMouseEvent *event)
{
    switch(m_mouseState)
    {
    case MouseState::Dragging:
        {
            auto deltaInChipCoordinates = m_viewPort.toViewport(event->pos()) 
                - m_viewPort.toViewport(m_mouseDownPos);

            m_viewPort.setViewportRect(m_viewPortRef - deltaInChipCoordinates);

            update();
        }
        break;
    case MouseState::None:
        break;
    };
}

void FloorplanView::wheelEvent(QWheelEvent *event)
{
    QPoint numPixels = event->pixelDelta();
    QPoint numDegrees = event->angleDelta() / 8;

    //FIXME: for 5.14 it is called position()
    const auto mousePos = event->pos();
    auto mousePosInViewport = m_viewPort.toViewport(mousePos);

    auto tmpViewPortRect = m_viewPort.getViewportRect();
    
    if (!numPixels.isNull()) 
    {
        tmpViewPortRect -= mousePosInViewport;
        if (numPixels.ry() > 0)
        {
            tmpViewPortRect.m_ll = ChipDB::Coord64{tmpViewPortRect.m_ll.m_x * 1.1, tmpViewPortRect.m_ll.m_y * 1.1};
            tmpViewPortRect.m_ur = ChipDB::Coord64{tmpViewPortRect.m_ur.m_x * 1.1, tmpViewPortRect.m_ur.m_y * 1.1};
            tmpViewPortRect += mousePosInViewport;
            m_viewPort.setViewportRect(tmpViewPortRect);
            update();
        }
        else
        {
            tmpViewPortRect.m_ll = ChipDB::Coord64{tmpViewPortRect.m_ll.m_x / 1.1, tmpViewPortRect.m_ll.m_y / 1.1};
            tmpViewPortRect.m_ur = ChipDB::Coord64{tmpViewPortRect.m_ur.m_x / 1.1, tmpViewPortRect.m_ur.m_y / 1.1};
            tmpViewPortRect += mousePosInViewport;
            m_viewPort.setViewportRect(tmpViewPortRect);
            update();
        }
    } 
    else if (!numDegrees.isNull()) 
    {
        tmpViewPortRect -= mousePosInViewport;
        QPoint numSteps = numDegrees / 15;
        if (numSteps.ry() > 0)
        {
            tmpViewPortRect.m_ll = ChipDB::Coord64{tmpViewPortRect.m_ll.m_x * 1.1, tmpViewPortRect.m_ll.m_y * 1.1};
            tmpViewPortRect.m_ur = ChipDB::Coord64{tmpViewPortRect.m_ur.m_x * 1.1, tmpViewPortRect.m_ur.m_y * 1.1};
            tmpViewPortRect += mousePosInViewport;
            m_viewPort.setViewportRect(tmpViewPortRect);           
            update();
        }
        else
        {
            tmpViewPortRect.m_ll = ChipDB::Coord64{tmpViewPortRect.m_ll.m_x / 1.1, tmpViewPortRect.m_ll.m_y / 1.1};
            tmpViewPortRect.m_ur = ChipDB::Coord64{tmpViewPortRect.m_ur.m_x / 1.1, tmpViewPortRect.m_ur.m_y / 1.1};
            tmpViewPortRect += mousePosInViewport;
            m_viewPort.setViewportRect(tmpViewPortRect);
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

    if (m_db == nullptr)
        return;

    drawRegions(painter);
    drawInstances(painter);

    if (m_overlay != nullptr)
    {
        m_overlay->paint(painter);
    }

#if 0
    // TODO: draw ruler

    // TODO: draw cells
    painter.setBrush(Qt::NoBrush);
    painter.setPen(Qt::darkGray);
    for(auto const ins : m_netlist->m_instances)
    {   
        drawInstance(painter, ins);
    }
#endif    
}

void FloorplanView::drawRegions(QPainter &p)
{
    if (m_db == nullptr)
    {
        return;
    }

    const QColor regionColor("#FF90EE90"); // light green

    p.setBrush(Qt::NoBrush);
    for(auto region : m_db->floorplan().m_regions)
    {
        auto regionRect     = m_viewPort.toScreen(region->m_rect);
        auto placementRect  = m_viewPort.toScreen(region->getPlacementRect());
        
        // draw outer region (with halo)
        if (!region->m_halo.isNull())
        {
            p.setPen(QPen(regionColor, 1, Qt::DashDotDotLine));
            p.drawRect(regionRect);
        }

        // draw placeable region
        p.setPen(QPen(regionColor, 1, Qt::SolidLine));
        p.drawRect(placementRect);

        drawRows(p, region);
    }
}

void FloorplanView::drawRows(QPainter &p, const ChipDB::Region *region)
{
    const QColor rowColor("#FFADD8E6"); // light blue

    p.setBrush(Qt::NoBrush);
    p.setPen(rowColor);
    
    for(auto const& row : region->m_rows)
    {
        auto rowRect = m_viewPort.toScreen(row.m_rect);
        p.drawRect(rowRect);
    }
}

void FloorplanView::drawCell(QPainter &p, const ChipDB::InstanceBase *ins)
{    
    QRectF cellRect;
    cellRect.setBottomLeft(m_viewPort.toScreen(ins->m_pos));
    cellRect.setTopRight(m_viewPort.toScreen(ins->m_pos + ins->instanceSize() ));

    // check if the instance is in view
    //if (!cellRect.intersects(m_viewPort))
    //    return;

    p.setPen(Qt::green);
    p.drawRect(cellRect);

    switch(ins->m_orientation)
    {
    case ChipDB::Orientation::R0: // aka North
        {
            auto p1 = cellRect.bottomLeft() - QPointF{0, cellRect.height() / 2};
            auto p2 = cellRect.bottomLeft() + QPointF{cellRect.width() / 2, 0};
            p.drawLine(p1, p2);
        }
        break;
    case ChipDB::Orientation::MX: // aka flipped South
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

void FloorplanView::drawPin(QPainter &p, const ChipDB::InstanceBase *ins)
{
    QRectF cellRect;
    cellRect.setBottomLeft(m_viewPort.toScreen(ins->m_pos));
    cellRect.setTopRight(m_viewPort.toScreen(ins->m_pos + ChipDB::Coord64{1000,1000}));

    // check if the instance is in view
    //if (!cellRect.intersects(m_viewPort))
    //    return;

    p.setPen(Qt::white);
    p.drawRect(cellRect);

    QFontMetrics fm(font());
    auto txtpoint = cellRect.center();
    txtpoint += {0, static_cast<qreal>(fm.height())};
    drawCenteredText(p, txtpoint, ins->m_name, font());
}

void FloorplanView::drawInstances(QPainter &p)
{
    if (m_db == nullptr)
    {
        return;
    }

    auto topModule = m_db->design().getTopModule();
    if (topModule == nullptr)
    {
        return;
    }

    for(auto ins : topModule->m_netlist->m_instances)
    {
        if ((ins != nullptr) && (ins->m_placementInfo != ChipDB::PlacementInfo::UNPLACED) && (ins->m_placementInfo != ChipDB::PlacementInfo::IGNORE))
        {
            switch(ins->m_insType)
            {
            case ChipDB::InstanceType::ABSTRACT:
                break;
            case ChipDB::InstanceType::PIN:
                drawPin(p, ins);
                break;
            case ChipDB::InstanceType::MODULE:
                break;
            case ChipDB::InstanceType::CELL:
                drawCell(p, ins);
                break;
            default:
                break;         
            }
        }
    }
}
