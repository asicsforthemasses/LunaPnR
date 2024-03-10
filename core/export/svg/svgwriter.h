// SPDX-FileCopyrightText: 2021-2024 Niels Moseley <asicsforthemasses@gmail.com>
//
// SPDX-License-Identifier: GPL-3.0-only

#pragma once
#include <iostream>

#include "database/database.h"

namespace LunaCore::SVG
{

class Writer
{
public:
    Writer(std::ostream &os, int64_t width, int64_t height);
    virtual ~Writer();

    void setViewport(const ChipDB::Rect64 &viewport);

    void setStrokeColour(uint32_t rgb);
    void setStrokeWidth(float w);
    void setFillColour(uint32_t rgb);

    void setFontFamily(const std::string &family);
    void setFontSize(float pt);

    void drawLine(const ChipDB::Coord64 &begin, const ChipDB::Coord64 &end);
    void drawCircle(const ChipDB::Coord64 &center, const ChipDB::CoordType radius);
    void drawRectangle(const ChipDB::Coord64 &p1, const ChipDB::Coord64 &p2);
    void drawRectangle(const ChipDB::Rect64 &r);
    void drawText(const ChipDB::Coord64 &pos, const std::string &text);
    void drawCenteredText(const ChipDB::Coord64 &pos, const std::string &text);
    void close();

protected:

    struct SVGCoord
    {
        SVGCoord() = default;
        SVGCoord(float x, float y) : m_x(x), m_y(y) {}
        SVGCoord(const ChipDB::Coord64 &p) : m_x(static_cast<float>(p.m_x)), m_y(static_cast<float>(p.m_y)) {}

        float m_x{0.0f};
        float m_y{0.0f};

        SVGCoord operator+(const SVGCoord &rhs) const
        {
            return SVGCoord{m_x + rhs.m_x, m_y + rhs.m_y};
        }

        SVGCoord operator-(const SVGCoord &rhs) const
        {
            return SVGCoord{m_x - rhs.m_x, m_y - rhs.m_y};
        }

    };

    SVGCoord toSVG(const ChipDB::Coord64 &p) const;
    SVGCoord toSVG(const ChipDB::CoordType &length) const;

    bool        m_closed;
    int64_t     m_width;    // svg width
    int64_t     m_height;   // svg height

    ChipDB::Rect64 m_viewport;

    std::string m_strokeColour;
    std::string m_fillColour;
    float       m_strokeWidth;
    float       m_fontSize;
    std::string m_fontFamily;

    std::ostream &m_os;
};

}; // namespace

