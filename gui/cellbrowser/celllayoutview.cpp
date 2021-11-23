
#include <QFontMetricsF>
#include <QPainter>
#include <QWheelEvent>
#include <QRectF>
#include <QPolygonF>
#include <algorithm>
#include <variant>
#include "celllayoutview.h"
#include "../common/guihelpers.h"

using namespace GUI;

// ********************************************************************************
//   CellLayoutView
// ********************************************************************************

CellLayoutView::CellLayoutView(QWidget *parent) : QWidget(parent),
    m_mouseState(MouseState::None), m_db(nullptr)
{
    m_viewport = {{-10000,-10000},{10000, 10000}};
}

CellLayoutView::~CellLayoutView()
{
}

void CellLayoutView::fixCoordinates(QPointF &p1, QPointF &p2)
{
    double x1 = p1.x();
    double y1 = p1.y();
    double x2 = p2.x();
    double y2 = p2.y();
    
    if (x1 > x2)
        std::swap(x1, x2);

    if (y1 > y2)
        std::swap(y1, y2);

    p1.setX(x1);
    p1.setY(y1);
    p2.setX(x2);
    p2.setY(y2);
}

void CellLayoutView::setDatabase(const Database *db)
{
    m_db = db;
    update();
}

void CellLayoutView::setCell(const ChipDB::Cell *cell)
{
    m_zoomLevel = 1;

    if (cell != nullptr)
    {
        m_cell = *cell;
        auto margin = static_cast<int64_t>(std::max(cell->m_size.m_x, cell->m_size.m_y) * 0.1);
        m_viewport = {{-margin,-margin}, cell->m_size + ChipDB::Coord64{margin,margin}};
    }
    else
    {
        m_cell.m_pins.clear();
        m_cell.m_obstructions.clear();
    }
}

ChipDB::Coord64 CellLayoutView::toChipDelta(const QPointF &delta) const noexcept
{
    if ((width() == 0) || (height() == 0))
    {
        return ChipDB::Coord64{0,0};
    }

    const auto sx = static_cast<double>(m_viewport.width()) / width();
    const auto sy = static_cast<double>(m_viewport.height()) / height();
    const auto dx = static_cast<int64_t>(delta.x() * sx);
    const auto dy = static_cast<int64_t>(delta.y() * sy);

    std::cout << ChipDB::Coord64{dx,dy} << "\n";

    return ChipDB::Coord64{dx,dy};
}

ChipDB::Coord64 CellLayoutView::toChip(const QPointF &p) const noexcept
{
    if ((width() == 0) || (height() == 0))
    {
        return ChipDB::Coord64{0,0};
    }

    const auto sx = static_cast<double>(m_viewport.width()) / width();
    const auto sy = static_cast<double>(m_viewport.height()) / height();
    const auto x = static_cast<int64_t>(p.x() * sx) + m_viewport.left();
    const auto y = static_cast<int64_t>(p.y() * sy) + m_viewport.bottom();

    return ChipDB::Coord64{x,y};
}

QPointF CellLayoutView::toScreen(const ChipDB::Coord64 &pos) const noexcept
{
    const auto sx = width() / static_cast<double>(m_viewport.width());
    const auto sy = height() / static_cast<double>(m_viewport.height());
    const auto localPos = pos - m_viewport.getLL();
    const auto x = localPos.m_x * sx;
    const auto y = localPos.m_y * sy;

    return QPointF{x,y};
}

void CellLayoutView::mousePressEvent(QMouseEvent *event)
{    
    setCursor(Qt::ClosedHandCursor);
    m_mouseState = MouseState::Dragging;
    m_mouseDownPos = event->pos();
    m_viewportStartDrag = m_viewport;
}

void CellLayoutView::mouseReleaseEvent(QMouseEvent *event)
{
    setCursor(Qt::ArrowCursor);
    m_mouseState = MouseState::None;
}

void CellLayoutView::mouseMoveEvent(QMouseEvent *event)
{
    auto offset = toChipDelta(m_mouseDownPos - event->pos());
    m_viewport = m_viewportStartDrag.movedBy(offset);
    update();
}

void CellLayoutView::wheelEvent(QWheelEvent *event)
{
    //QPoint numPixels = event->pixelDelta();
    QPoint numDegrees = event->angleDelta() / 8;

    auto mouseChipPos = toChip(event->pos());
    std::cout << mouseChipPos << "\n";

    if (numDegrees.y() > 0)
    {
        // limit zoom level to 50
        //m_zoomLevel = std::min(m_zoomLevel+1, 50);
        auto llx = mouseChipPos.m_x - ((mouseChipPos.m_x - m_viewport.m_ll.m_x) * 80 / 100);
        auto lly = mouseChipPos.m_y - ((mouseChipPos.m_y - m_viewport.m_ll.m_y) * 80 / 100);
        auto urx = mouseChipPos.m_x + ((m_viewport.m_ur.m_x - mouseChipPos.m_x) * 80 / 100);
        auto ury = mouseChipPos.m_y + ((m_viewport.m_ur.m_y - mouseChipPos.m_y) * 80 / 100);

        m_viewport.setLL({llx, lly});
        m_viewport.setUR({urx, ury});

        update();   
    }
    else if (numDegrees.y() < 0)
    {
        // limit zoom level to 1
        m_zoomLevel = std::max(m_zoomLevel-1, 1);    

        auto llx = mouseChipPos.m_x - ((mouseChipPos.m_x - m_viewport.m_ll.m_x) * 100 / 80);
        auto lly = mouseChipPos.m_y - ((mouseChipPos.m_y - m_viewport.m_ll.m_y) * 100 / 80);
        auto urx = mouseChipPos.m_x + ((m_viewport.m_ur.m_x - mouseChipPos.m_x) * 100 / 80);
        auto ury = mouseChipPos.m_y + ((m_viewport.m_ur.m_y - mouseChipPos.m_y) * 100 / 80);

        m_viewport.setLL({llx, lly});
        m_viewport.setUR({urx, ury});

        update();        
    }    

    event->accept();
}

void CellLayoutView::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);

    painter.setRenderHint(QPainter::Antialiasing, true);
    painter.setBrush(Qt::black);
    painter.setPen(Qt::black);

    painter.fillRect(rect(), Qt::black);

    //FIXME: when the cell exists but does not contain any
    // physical information, like when only the .lib was loaded
    // tell the user there is no geometry information

    // draw cell boundary 0,0 - m_size
    ChipDB::Coord64 pp = {0,0};
    QPointF p1 = toScreen(pp);
    QPointF p2 = toScreen(m_cell.m_size);

    painter.setPen(QColor(255,255,255,255));
    painter.drawRect(p1.x(),p1.y(),p2.x()-p1.x(),p2.y()-p1.y());

    // draw the cell name at the top
    drawCenteredText(painter, QPointF(width()/2, 20), m_cell.m_name, font());

    // draw the horizontal dimensions of the cell
    //
    // toScreen leaves 10% of the widget size for dimensional
    // drawing
    // 
    const float dimWidth = std::min(width() * 0.1f, height() * 0.1f);
    const float d2 = dimWidth / 2.0f;
    QRectF txtrect(p1.x(), height() - dimWidth, p2.x() - p1.x(), dimWidth);
    painter.drawLine(p1.x(), height() - d2, p2.x(), height() - d2);
    painter.drawLine(p1.x(), height() - dimWidth+2, p1.x(), height()-2);
    painter.drawLine(p2.x(), height() - dimWidth+2, p2.x(), height()-2);

    // convert int32_t nanometers to microns
    const int64_t nm = m_cell.m_size.m_x % 1000;
    const int64_t um = m_cell.m_size.m_x / 1000;

    char txtbuf[20];
    snprintf(txtbuf, sizeof(txtbuf), "%ld.%03ld um", um, nm);
    
    drawCenteredText(painter, txtrect.center(), txtbuf, font(), Qt::black);

    // draw the vertical dimensions of the cell
    QRectF txtrect2(0, p1.y(), dimWidth, p2.y());
    painter.drawLine(d2, p1.y(), d2, p2.y());
    painter.drawLine(2, p1.y(), dimWidth-2, p1.y());
    painter.drawLine(2, p2.y(), dimWidth-2, p2.y());

    // convert int32_t nanometers to microns
    const int64_t nm2 = m_cell.m_size.m_y % 1000;
    const int64_t um2 = m_cell.m_size.m_y / 1000;

    snprintf(txtbuf, sizeof(txtbuf), "%ld.%03ld um", um2, nm2);
    QPointF txtpoint(2, (p1.y()+p2.y())/2);
    drawLeftText(painter, txtpoint, txtbuf, font(), Qt::black);

    // draw pins
    size_t pinIndex = 0;
    for(auto pin : m_cell.m_pins)
    {
        auto const& layout = pin->m_pinLayout;

        for(auto const& layer : layout)
        {
            auto info = getLayerRenderInfo(layer.first);
            if (info)
            {
                QBrush brush(info->routing().getColorPixmap());
                brush.setColor(info->routing().getColor());
                drawGeometry(painter, layer.second, brush);
            }
            else
            {
                drawGeometry(painter, layer.second, QColor(255,255,255,80) /* default color / fill */);
            }
        }
    }

    // paint all obstructions
    painter.setPen(Qt::red);
    for(auto const& layer : m_cell.m_obstructions)
    {
        auto info = getLayerRenderInfo(layer.first);
        if (info)
        {
            std::string obstructionLayerName = layer.first;
            obstructionLayerName.append(":OBS");

            QBrush brush(info->obstruction().getColorPixmap());
            brush.setColor(info->obstruction().getColor());
            drawGeometry(painter, layer.second, brush);
        }
        else
        {
            drawGeometry(painter, layer.second, QBrush(QColor(255,0,0,80)));
        }
    }

    // draw all the text for the pins
    for(auto pin : m_cell.m_pins)
    {
        // don't display power and ground pins
        // FIXME: make this configurable
        if ((pin->m_iotype == ChipDB::IOType::GROUND) || 
            (pin->m_iotype == ChipDB::IOType::POWER))
        {
            continue;
        }

        for(auto const& layer : pin->m_pinLayout)
        {
            auto layerObjects = layer.second;
            for(auto const &obj : layerObjects)
            {
                // FIXME: we only support rectangles for now..
                if (obj.index() == 0 /* rectangle */)
                {
                    auto txtRect = std::get<ChipDB::Rectangle>(obj);
                    QRectF screenRect(toScreen(txtRect.m_rect.getLL()), toScreen(txtRect.m_rect.getUR()));
                    painter.setPen(Qt::white);
                    drawCenteredText(painter, screenRect.center() , pin->m_name, font(), Qt::NoBrush);
                }
            }
        }
    }
}

void CellLayoutView::drawGeometry(QPainter &painter, const ChipDB::GeometryObjects &objs,
    const QBrush &brush) const
{
    painter.setBrush(brush);
    painter.setPen(brush.color());

    for(auto const& obj : objs)
    {
        switch(obj.index())
        {
        case 0: // rectangle
            drawGeometry(painter, std::get<ChipDB::Rectangle>(obj));
            break;
        case 1: // polygon
            drawGeometry(painter, std::get<ChipDB::Polygon>(obj));
            break;
        }
    }
}

void CellLayoutView::drawGeometry(QPainter &painter, const ChipDB::Rectangle &r) const
{
    QRectF screenRect(toScreen(r.m_rect.getLL()), toScreen(r.m_rect.getUR()));

    // transform the brush so the texture moves with the rectangle.
    QTransform transf;
    transf.translate(screenRect.left(), screenRect.top());
    auto b = painter.brush();
    b.setTransform(transf);
    painter.setBrush(b);
    
    painter.drawRect(screenRect);
}

void CellLayoutView::drawGeometry(QPainter &painter, const ChipDB::Polygon &r) const
{
    QPolygonF poly;

    poly.resize(r.m_points.size());

    for(size_t idx=0; idx<r.m_points.size(); idx++)
    {
        poly[idx] = toScreen(r.m_points.at(idx));
    }
    
    painter.drawPolygon(poly);
}

LayerRenderInfo* CellLayoutView::getLayerRenderInfo(const std::string &layerName) const
{
    if (m_db == nullptr)
    {
        return nullptr;
    }

    return m_db->m_layerRenderInfoDB.lookup(layerName);
}
