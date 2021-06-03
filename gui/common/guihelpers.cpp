#include "guihelpers.h"

void GUI::drawCenteredText(QPainter &painter, const QPointF &pos, const std::string &txt,
    const QFont &font,
    const QBrush &style)
{
    QString str = QString::fromStdString(txt);
    QPointF pp = pos;
    QFontMetricsF fm(font);

    qreal pixelsWide = fm.horizontalAdvance(str);
    qreal pixelsHigh = fm.xHeight();    // ascent() is too large..

    pp += QPointF(-pixelsWide/2.0f, pixelsHigh/2.0f);
    QRectF fillRect(pp - QPointF(0,fm.ascent()), QSize(pixelsWide, fm.height()));
    painter.fillRect(fillRect, style);
    painter.drawText(pp, str);
}


void GUI::drawLeftText(QPainter &painter, const QPointF &pos, const std::string &txt,
    const QFont &font,
    const QBrush &style)
{
    QString str = QString::fromStdString(txt);
    QPointF pp = pos;
    QFontMetricsF fm(font);

    qreal pixelsWide = fm.horizontalAdvance(str);
    qreal pixelsHigh = fm.xHeight();    // ascent() is too large..

    pp += QPointF(0.0f, pixelsHigh/2.0f);
    QRectF fillRect(pp - QPointF(0,fm.ascent()), QSize(pixelsWide, fm.height()));
    painter.fillRect(fillRect, style);
    painter.drawText(pp, str);    
}
