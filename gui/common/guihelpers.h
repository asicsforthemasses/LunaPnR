#pragma once

#include<string>
#include<QPainter>
#include<QColor>
#include<QPixmap>

namespace GUI
{

void drawCenteredText(QPainter &painter, const QPointF &pos, const std::string &txt,
    const QFont &font,
    const QBrush &brush = Qt::NoBrush);

void drawLeftText(QPainter &painter, const QPointF &pos, const std::string &txt,
    const QFont &font,
    const QBrush &brush = Qt::NoBrush);

class AlternatingColors
{
public:
    AlternatingColors() : m_state(true) {}

    AlternatingColors(const QColor &col1, const QColor &col2) : m_state(true)
    {
        setColors(col1, col2);
    }

    void resetState()
    {
        m_state = true;
    }

    void setColors(const QColor &col1, const QColor &col2)
    {
        m_colors[0] = col1;
        m_colors[1] = col2;
    }

    QColor getColor() const
    {
        return m_state ? m_colors[0] : m_colors[1];
    }

    constexpr void update()
    {
        m_state = !m_state;
    }

    QColor getColorAndUpdate()
    {
        auto col = getColor();
        update();
        return col;
    }

protected:
    bool   m_state;
    QColor m_colors[2];
};

std::optional<QPixmap> createPixmapFromString(const std::string &pixels, int width, int height);
std::string pixmapToString(const QPixmap &p);

};
