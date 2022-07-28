#include "clickablelabel.h"

using namespace GUI;

ClickableLabel::ClickableLabel(QWidget* parent) : QLabel(parent)
{    
}

ClickableLabel::ClickableLabel(const QString &text, QWidget* parent) : QLabel(text, parent)
{
}

void ClickableLabel::mousePressEvent(QMouseEvent* event)
{
    emit clicked();
}
