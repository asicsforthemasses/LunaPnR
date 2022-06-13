// SPDX-FileCopyrightText: 2021-2022 Niels Moseley, <n.a.moseley@moseleyinstruments.com>, et al.
// SPDX-FileCopyrightText: 2022 Niels Moseley <asicsforthemasses@gmail.com>
//
// SPDX-License-Identifier: GPL-3.0-only

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
    size_t numPixels = width*height;
    int x=0;
    int y=0;
    while(numPixels > 0)
    {
        char c = pixels.at(ofs);
        if (c == '\n')
        {
            ofs++;
            continue;
        }

        if ((c != ' ') && (c != '.'))
        {
            image.setPixel(QPoint(x,y), 1);
        }
        else
        {
            image.setPixel(QPoint(x,y), 0);
        }

        x++;
        if (x == width)
        {
            x = 0;
            y++;
        }
        
        numPixels--;
        ofs++;
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

void GUI::Viewport::setScreenRect(const QRect &screen) noexcept
{
    m_screen = screen;
    updatePrecomputedConstants();
}

void GUI::Viewport::setViewportRect(const ChipDB::Rect64 &viewport) noexcept
{
    m_viewport = viewport;
    updatePrecomputedConstants();
}

void GUI::Viewport::updatePrecomputedConstants() noexcept
{
    auto vpSize = m_viewport.getSize();
    auto scSize = m_screen.size();

    if ((vpSize.m_x <= 0) || (vpSize.m_y <= 0))
    {
        m_valid = false;
        return;
    }

    if (scSize.isNull())
    {
        m_valid = false;
        return;
    }

    m_sx = static_cast<float>(scSize.width()) / vpSize.m_x;
    m_sy = static_cast<float>(scSize.height()) / vpSize.m_y;

    m_valid = true;
}

bool GUI::Viewport::isValid() const noexcept
{
    return m_valid;
}

QRectF GUI::Viewport::toScreen(const ChipDB::Rect64 &rect) const noexcept
{
    const auto ul = toScreen(rect.getUL());
    const auto lr = toScreen(rect.getLR());

    return QRectF{ul,lr};
    //return QRectF{toScreen(rect.getUL()), toScreen(rect.getUR())};
}

QPointF GUI::Viewport::toScreen(const ChipDB::Coord64 &point) const noexcept
{
    if (m_valid)
    {
        const float x = m_screen.left() + static_cast<float>(point.m_x - m_viewport.left())*m_sx;
        const float y = m_screen.top() - static_cast<float>(point.m_y - m_viewport.top())*m_sy;

        return QPointF{x,y};
    }
    else
    {
        return QPointF{0.0f,0.0f};
    }
}

ChipDB::Rect64  GUI::Viewport::toViewport(const QRectF &rect) const noexcept
{
    const ChipDB::Coord64 ll(static_cast<int64_t>(rect.bottomLeft().x()), 
        static_cast<int64_t>(rect.bottomLeft().y()));

    const ChipDB::Coord64 ur(static_cast<int64_t>(rect.topRight().x()),
        static_cast<int64_t>(rect.topRight().y()));

    return ChipDB::Rect64{ll, ur};
}

ChipDB::Rect64  GUI::Viewport::toViewport(const QRect &rect) const noexcept
{
    const ChipDB::Coord64 ll(rect.bottomLeft().x(), rect.bottomLeft().y());
    const ChipDB::Coord64 ur(rect.topRight().x(), rect.topRight().y());
    return ChipDB::Rect64{ll, ur};
}

ChipDB::Coord64 GUI::Viewport::toViewport(const QPointF &point) const noexcept
{
    // const float x = m_screen.left() + static_cast<float>(point.m_x - m_viewport.left())*m_sx;
    // const float y = m_screen.top() - static_cast<float>(point.m_y - m_viewport.top())*m_sy;
    if (m_valid)
    {
        const float x = m_viewport.left() + (point.x() - m_screen.left()) / m_sx;
        const float y = m_viewport.top()  - (point.y() - m_screen.top()) / m_sy;
        return ChipDB::Coord64{static_cast<int64_t>(x), static_cast<int64_t>(y)};
    }
    else 
    {
        return ChipDB::Coord64{0,0};
    }
}

ChipDB::Coord64 GUI::Viewport::toViewport(const QPoint &point) const noexcept
{
    if (m_valid)
    {
        const float x = m_viewport.left() + (point.x() - m_screen.left()) / m_sx;
        const float y = m_viewport.top()  - (point.y() - m_screen.top()) / m_sy;
        return ChipDB::Coord64{static_cast<int64_t>(x), static_cast<int64_t>(y)};    
    }
    else 
    {
        return ChipDB::Coord64{0,0};
    }    
}
