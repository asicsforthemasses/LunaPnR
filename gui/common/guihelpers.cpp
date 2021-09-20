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

std::optional<QPixmap> GUI::createPixmapFromString(const std::string &pixels, int width, int height)
{
    QImage image(width, height, QImage::Format_Mono);
    image.setColor(0, QColor("#000000").rgb());
    image.setColor(1, QColor("#FFFFFF").rgb());

    if (pixels.size() < (width*height))
    {
        //doLog(LOG_ERROR, "Cannot set texture from string: not enough characters\n");
        return std::nullopt;
    }

    size_t ofs = 0;
    for(int y=0; y<height; y++)
    {
        for(int x=0; x<width; x++)
        {
            // skip EOL
            if (pixels.at(ofs) == '\n')
                continue;

            if (pixels.at(ofs) != ' ')
            {
                image.setPixel(QPoint(x,y), 1);
            }
            else
            {
                image.setPixel(QPoint(x,y), 0);
            }
            ofs++;
        }
    }

    QPixmap pixmap;
    bool result = pixmap.convertFromImage(image, Qt::NoFormatConversion);
    if (result)
    {
        return pixmap;
    }

    return std::nullopt;
}

std::string GUI::pixmapToString(const QPixmap &p)
{
    std::string pattern;

    if (p.isNull())
    {
        return "";
    }

    QImage image = p.toImage();

    if (image.isNull())
    {
        return "";
    }

    for(int y=0; y < image.height(); y++)
    {
        for(int x=0; x < image.width(); x++)
        {
            auto pixel = image.pixelColor(x,y);
            if (pixel == QColor("#000000"))
            {
                pattern += ' ';
            }
            else
            {
                pattern += '*';
            }            
        }
        pattern += '\n';
    }

    return pattern;
}
