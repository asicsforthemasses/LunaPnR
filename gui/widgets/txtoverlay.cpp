#include <iostream>
#include "txtoverlay.h"

using namespace GUI;

void TxtOverlay::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);

    // detect a text size change
    QFontMetrics fm(painter.font());
    auto txtRect = fm.boundingRect(m_txt);
    txtRect = txtRect.marginsAdded(m_margins);
    if (txtRect != m_txtRect)
    {
        m_txtRect = txtRect;
        resize(m_txtRect.size());
    }

    QColor bkcolor("#C0333333");
    QColor txtcolor("#FFFFFFFF");

    painter.setBrush(bkcolor);
    painter.setPen(bkcolor);
    painter.drawRoundedRect(rect(),4,4);
    painter.setPen(txtcolor);

    painter.drawText(rect(), Qt::AlignVCenter | Qt::AlignHCenter, m_txt);
}

void TxtOverlay::setText(const QString &txt)
{
    m_txt = txt;
}
