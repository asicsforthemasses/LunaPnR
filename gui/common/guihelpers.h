#pragma once

#include<string>
#include<QPainter>

namespace GUI
{

void drawCenteredText(QPainter &painter, const QPointF &pos, const std::string &txt,
    const QFont &font,
    const QBrush &brush = Qt::NoBrush);

void drawLeftText(QPainter &painter, const QPointF &pos, const std::string &txt,
    const QFont &font,
    const QBrush &brush = Qt::NoBrush);

};
