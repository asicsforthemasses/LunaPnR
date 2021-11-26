#include "svgwriter.h"


LunaCore::SVG::Writer::Writer(std::ostream &os, int64_t width, int64_t height) : m_os(os), m_width(width), m_height(height)
{
    m_os << R"(<svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 )";
    m_os << m_width << " " << m_height << "\">\n"; 
    m_closed = false;

    m_viewport = ChipDB::Rect64{{0,0}, {width, height}};
    m_strokeWidth = 1;
    m_strokeColour = "FFFFFFFF";
    m_fillColour   = "FFFFFFFF";
    m_fontSize     = 8;
    m_fontFamily   = "Arial, Helvetica, sans-serif";
};

LunaCore::SVG::Writer::~Writer()
{
    if (!m_closed)
    {
        close();
    }
}

void LunaCore::SVG::Writer::close()
{
    m_os << "</svg>\n";
    m_closed = true;
}

void LunaCore::SVG::Writer::setViewport(const ChipDB::Rect64 &viewport)
{
    m_viewport = viewport;
}

LunaCore::SVG::Writer::SVGCoord LunaCore::SVG::Writer::toSVG(const ChipDB::Coord64 &p) const
{
    auto pp = p - m_viewport.m_ll;
    float xmul = m_width / static_cast<float>(m_viewport.width());
    float ymul = m_height / static_cast<float>(m_viewport.height());

    return SVGCoord{pp.m_x * xmul, pp.m_y * ymul};
}

LunaCore::SVG::Writer::SVGCoord LunaCore::SVG::Writer::toSVG(const ChipDB::CoordType &length) const
{
    float xmul = m_width / static_cast<float>(m_viewport.width());
    float ymul = m_height / static_cast<float>(m_viewport.height());    
    return SVGCoord{length * xmul, length * ymul};
}

void LunaCore::SVG::Writer::setStrokeColour(uint32_t rgb)
{
    char buffer[10];
    snprintf(buffer, sizeof(buffer), "%08X", rgb);
    m_strokeColour = buffer;
}

void LunaCore::SVG::Writer::setStrokeWidth(float w)
{
    m_strokeWidth = w;
}

void LunaCore::SVG::Writer::setFillColour(uint32_t rgb)
{
    char buffer[10];
    snprintf(buffer, sizeof(buffer), "%08X", rgb);
    m_fillColour = buffer;
}

void LunaCore::SVG::Writer::setFontSize(float pt)
{
    m_fontSize = pt;
}

void LunaCore::SVG::Writer::setFontFamily(const std::string &family)
{
    m_fontFamily = family;
}

void LunaCore::SVG::Writer::drawLine(const ChipDB::Coord64 &begin, const ChipDB::Coord64 &end)
{
    auto b = toSVG(begin);
    auto e = toSVG(end);

    m_os << "<line x1=\"" << b.m_x << "\" y1=\"" << b.m_y <<  "\" ";
    m_os << "x2=\"" << e.m_x << "\" y2=\"" << e.m_y <<  "\" ";
    m_os << "style=\"stroke: #" << m_strokeColour << "; ";
    m_os << "stroke-width: " << m_strokeWidth << "; \"/>\n";
}

void LunaCore::SVG::Writer::drawCircle(const ChipDB::Coord64 &center, const ChipDB::CoordType radius)
{
    SVGCoord c = toSVG(center);
    SVGCoord r = toSVG(radius);
            
    m_os << "<ellipse cx=\"" << c.m_x << "\" cy=\"" << c.m_y <<  "\" rx=\"" << r.m_x << "\" ry=\"" << r.m_y;
    m_os << "\" style=\"fill:#" << m_fillColour << "; ";
    m_os << "stroke: #" << m_strokeColour << "; ";
    m_os << "stroke-width: " << m_strokeWidth << "; \"/>\n";
}

void LunaCore::SVG::Writer::drawRectangle(const ChipDB::Coord64 &p1, const ChipDB::Coord64 &p2)
{
    SVGCoord pp1 = toSVG(p1);
    SVGCoord pp2 = toSVG(p2);

    float x = std::min(pp1.m_x, pp2.m_x);
    float y = std::min(pp1.m_y, pp2.m_y);
    float w = std::abs(std::max(pp1.m_x, pp2.m_x) - x);
    float h = std::abs(std::max(pp1.m_y, pp2.m_y) - y);

    m_os << "<rect x=\"" << x << "\" y=\"" << y << "\"";
    m_os << " width=\"" << w << "\" ";
    m_os << " height=\"" << h << "\" ";
    m_os << "style=\"stroke: #" << m_strokeColour << "; ";
    m_os << "fill:#" << m_fillColour << "; ";
    m_os << "stroke-width: " << m_strokeWidth << "; \"/>\n";
}

void LunaCore::SVG::Writer::drawRectangle(const ChipDB::Rect64 &r)
{
    drawRectangle(r.m_ll, r.m_ur);
}

void LunaCore::SVG::Writer::drawText(const ChipDB::Coord64 &pos, const std::string &text)
{
    SVGCoord p = toSVG(pos);
    m_os << "<text x=\""  << p.m_x << "\" y=\"" << p.m_y << "\" ";
    m_os << "style=\"stroke:#" << m_strokeColour << "; fill:#" << m_strokeColour << "\" ";
    m_os << "font-family=\"" << m_fontFamily << "\" ";
    m_os << "font-size=\"" << m_fontSize << "\"";
    m_os << ">" << text << "</text>\n";
}

void LunaCore::SVG::Writer::drawCenteredText(const ChipDB::Coord64 &pos, const std::string &text)
{
    SVGCoord p = toSVG(pos);
    m_os << "<text x=\""  << p.m_x << "\" y=\"" << p.m_y << "\" ";
    m_os << "style=\"stroke:#" << m_strokeColour << "; fill:#" << m_strokeColour << "\" ";
    m_os << "font-size=\"" << m_fontSize << "\" ";
    m_os << "font-family=\"" << m_fontFamily << "\" ";
    m_os << R"(dominant-baseline="middle" text-anchor="middle" )";
    m_os << ">" << text << "</text>\n";    
}
