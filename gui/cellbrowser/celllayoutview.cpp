
#include <QFontMetricsF>
#include <QPainter>
#include <algorithm>
#include "celllayoutview.h"
#include "../common/guihelpers.h"

using namespace GUI;

// ********************************************************************************
//   DrawObstructionVisitor
// ********************************************************************************


DrawObstructionVisitor::DrawObstructionVisitor(CellLayoutView *view, QPainter *painter) 
    : m_view(view), m_painter(painter) 
{
}

#if 0
void DrawObstructionVisitor::visit(const ChipDB::CellRect *rect)
{
    QPointF p1 = m_view->toScreen(rect->m_rect.m_ll);
    QPointF p2 = m_view->toScreen(rect->m_rect.m_ur);
    m_view->fixCoordinates(p1,p2);
    QRectF r(p1,p2);

    auto col = QColor(128, 0, 0, 128);  // transparent red
    //Qt::BrushStyle style = Qt::SolidPattern;
    Qt::BrushStyle style = Qt::DiagCrossPattern;

    //FIXME: determine layer and set rendering attributes
    #if 0
    if ((obs.m_layerIdx < m_db->m_layerInfo.size()) && (obs.m_layerIdx >= 0))
    {
        col.setRed(m_db->m_layerInfo[obs.m_layerIdx].m_obsColor.r);
        col.setGreen(m_db->m_layerInfo[obs.m_layerIdx].m_obsColor.g);
        col.setBlue(m_db->m_layerInfo[obs.m_layerIdx].m_obsColor.b);
        col.setAlpha(m_db->m_layerInfo[obs.m_layerIdx].m_obsColor.a);
        style = toBrushStyle(m_db->m_layerInfo[obs.m_layerIdx].m_obsFillStyle);
    }
    #endif

    QBrush br(col, style);
    //m_painter->fillRect(r, br);
    m_painter->setBrush(br);
    m_painter->setPen(col);
    m_painter->drawRect(r);
}

void DrawObstructionVisitor::visit(const ChipDB::CellPolygon *poly)
{

}
#endif

// ********************************************************************************
//   DrawLayoutItemVisitor
// ********************************************************************************

DrawLayoutItemVisitor::DrawLayoutItemVisitor(CellLayoutView *view, QPainter *painter)
    : m_view(view), m_painter(painter), m_largestArea(0)
{
}

#if 0
void DrawLayoutItemVisitor::visit(const ChipDB::CellRect *rect)
{
    QPointF p1 = m_view->toScreen(rect->m_rect.m_ll);
    QPointF p2 = m_view->toScreen(rect->m_rect.m_ur);
    m_view->fixCoordinates(p1,p2);
    QRectF r(p1,p2);

    auto col = QColor(0, 0, 128, 128);
    Qt::BrushStyle style = Qt::SolidPattern;
    #if 0
    if ((pinrect.m_layerIdx < m_db->m_layerInfo.size()) && (pinrect.m_layerIdx >= 0))
    {
        col.setRed(m_db->m_layerInfo[pinrect.m_layerIdx].m_color.r);
        col.setGreen(m_db->m_layerInfo[pinrect.m_layerIdx].m_color.g);
        col.setBlue(m_db->m_layerInfo[pinrect.m_layerIdx].m_color.b);
        col.setAlpha(m_db->m_layerInfo[pinrect.m_layerIdx].m_color.a);
        style = toBrushStyle(m_db->m_layerInfo[pinrect.m_layerIdx].m_fillStyle);
    }
    #endif
    
    QBrush br(col, style);
    //painter.fillRect(r, br);
    m_painter->setBrush(br);
    m_painter->setPen(col);
    m_painter->drawRect(r);                

    // determine area of rectangle
    float area = (p2.x()-p1.x())*(p2.x()-p1.x()) + (p2.y()-p1.y())*(p2.y()-p1.y());
    if (area > m_largestArea)
    {
        m_largestArea = area;
        m_txtrect = r;
    }
}

void DrawLayoutItemVisitor::visit(const ChipDB::CellPolygon *poly)
{

}

#endif

QRectF DrawLayoutItemVisitor::getTextRect() const
{
    return m_txtrect;
}

// ********************************************************************************
//   CellLayoutView
// ********************************************************************************

CellLayoutView::CellLayoutView(QWidget *parent) : QWidget(parent), m_cell(nullptr)
{
}

CellLayoutView::~CellLayoutView()
{
}

#if 0
Qt::BrushStyle CellLayoutView::toBrushStyle(CellDB::LayerInfo::fillStyle_t s) const
{
    Qt::BrushStyle style = Qt::SolidPattern;
    switch(s)
    {
    case CellDB::LayerInfo::FS_SOLID:
        style = Qt::SolidPattern;
        break;
    case CellDB::LayerInfo::FS_HORIZONTAL:
        style = Qt::HorPattern;
        break;
    case CellDB::LayerInfo::FS_FDIAG:
        style = Qt::FDiagPattern;
        break;
    case CellDB::LayerInfo::FS_CROSS:
        style = Qt::CrossPattern;
        break;
    case CellDB::LayerInfo::FS_BDIAG:
        style = Qt::BDiagPattern;
        break;
    default:
        break;
    }
    return style;
}
#endif

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

void CellLayoutView::setCell(const ChipDB::Cell *cell)
{
    m_cell = cell;
}

QPointF CellLayoutView::toScreen(const ChipDB::Coord64 &pos)
{
    const float zoom = 1.0f;

    if (m_cell != nullptr)
    {
        float sx = 0.8*width() / m_cell->m_size.m_x;
        float sy = 0.8*height() / m_cell->m_size.m_y;
        
        float scale = std::min(sx,sy);

        float cellPixels_x = scale * m_cell->m_size.m_x;
        float cellPixels_y = scale * m_cell->m_size.m_y;

        float x = scale*pos.m_x + width()/2.0 - (cellPixels_x/2.0);
        float y = -scale*pos.m_y + height()/2.0 + (cellPixels_y/2.0);

        return QPointF(x, y);
    }
    
    return QPointF(0.0,0.0);
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

    if (m_cell != nullptr)
    {
        // draw cell boundary 0,0 - m_size
        ChipDB::Coord64 pp = {0,0};
        QPointF p1 = toScreen(pp);
        QPointF p2 = toScreen(m_cell->m_size);

        painter.setPen(QColor(255,255,255,255));
        painter.drawRect(p1.x(),p1.y(),p2.x()-p1.x(),p2.y()-p1.y());

        // draw the cell name at the top
        drawCenteredText(painter, QPointF(width()/2, 20), m_cell->m_name, font());

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
        const int64_t nm = m_cell->m_size.m_x % 1000;
        const int64_t um = m_cell->m_size.m_x / 1000;

        char txtbuf[20];
        snprintf(txtbuf, sizeof(txtbuf), "%ld.%03ld um", um, nm);
        
        drawCenteredText(painter, txtrect.center(), txtbuf, font(), Qt::black);

        // draw the vertical dimensions of the cell
        QRectF txtrect2(0, p1.y(), dimWidth, p2.y());
        painter.drawLine(d2, p1.y(), d2, p2.y());
        painter.drawLine(2, p1.y(), dimWidth-2, p1.y());
        painter.drawLine(2, p2.y(), dimWidth-2, p2.y());

#if 1
        // convert int32_t nanometers to microns
        const int64_t nm2 = m_cell->m_size.m_y % 1000;
        const int64_t um2 = m_cell->m_size.m_y / 1000;

        snprintf(txtbuf, sizeof(txtbuf), "%ld.%03ld um", um2, nm2);
        QPointF txtpoint(2, (p1.y()+p2.y())/2);
        drawLeftText(painter, txtpoint, txtbuf, font(), Qt::black);
#endif

        // draw pins
#if 0        
        size_t pinIndex = 0;
        for(auto const& pin : m_cell->pins())
        {
            auto layout = m_cell->pinLayout(pinIndex);

            DrawLayoutItemVisitor v(this, &painter);
            auto iter = layout->begin();
            while(iter != layout->end())
            {
                (*iter)->accept(&v);
                iter++;
            }

            auto txtRect=v.getTextRect();

            // display text in largest pin rectangle
            // FIXME: pre-process the text location and put it in the cell data structure?
            painter.setPen(Qt::white);            
            //painter.drawText(txtRect, Qt::AlignCenter, QString::fromStdString(pin.m_name));
            drawCenteredText(&painter, txtRect.center(), pin.getName(), font(), Qt::NoBrush);
            //painter.setBrush(Qt::NoBrush);
            //painter.drawRect(txtRect);
            pinIndex++;
        }

        // draw obstructions using the visitor pattern
        DrawObstructionVisitor v(this, &painter);
        for(auto const& obs : m_cell->obstructions())
        {
            obs->accept(&v);           
        }
#endif        
    }
}

