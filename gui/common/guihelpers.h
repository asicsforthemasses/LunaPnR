/*
  LunaPnR Source Code
  
  SPDX-License-Identifier: GPL-3.0-only
  SPDX-FileCopyrightText: 2022 Niels Moseley <asicsforthemasses@gmail.com>
*/


#pragma once

#include "common/dbtypes.h"
#include<string>
#include<optional>
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

/** class to handle viewport -> screen and vice versa transformations */
class Viewport
{
public:
    Viewport() : m_valid(false) {}

    void setScreenRect(const QRect &screen) noexcept;
    void setViewportRect(const ChipDB::Rect64 &viewport) noexcept;

    constexpr ChipDB::Rect64 getViewportRect() const noexcept
    {
        return m_viewport;
    }

    QRectF  toScreen(const ChipDB::Rect64 &rect) const noexcept;
    QPointF toScreen(const ChipDB::Coord64 &point) const noexcept;
    
    ChipDB::Rect64  toViewport(const QRectF &rect) const noexcept;
    ChipDB::Rect64  toViewport(const QRect &rect) const noexcept;
    ChipDB::Coord64 toViewport(const QPointF &point) const noexcept;
    ChipDB::Coord64 toViewport(const QPoint &point) const noexcept;

    bool isValid() const noexcept;

    constexpr auto getScreenRect() const noexcept
    {
        return m_screen;
    }

    constexpr auto getViewportRext() const noexcept
    {
        return m_viewport;
    }

protected:
    void updatePrecomputedConstants() noexcept;

    QRect m_screen;
    ChipDB::Rect64 m_viewport;

    float m_sx;
    float m_sy;
    bool  m_valid;
};

};
