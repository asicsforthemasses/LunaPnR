/*

    FlowManager Widget Source Code

    SPDX-License-Identifier: GPL-3.0-only
    SPDX-FileCopyrightText: 2022 Niels Moseley <asicsforthemasses@gmail.com>

*/

#include <QStyleOption>
#include <QPainter>
#include "flattogglebutton.h"

using namespace GUI;

FlatToggleButton::FlatToggleButton(QWidget *parent)
    : QAbstractButton(parent)
{
    m_offPixmap = std::make_unique<QPixmap>("://images/folded.png");
    m_onPixmap  = std::make_unique<QPixmap>("://images/unfolded.png");

    if ((m_onPixmap) && (!m_onPixmap->isNull()))
        setMinimumSize(m_onPixmap->size());

    setCheckable(true);
}

void FlatToggleButton::paintEvent(QPaintEvent *event)
{
    // make sure we have at least a little stylesheet support..
    QStyleOption opt;
    opt.init(this);

    QPainter painter(this);
    style()->drawPrimitive(QStyle::PE_Widget, &opt, &painter, this);

    bool state = isChecked();
    if (state)
    {
        if ((m_onPixmap) && (!m_onPixmap->isNull()))
            painter.drawPixmap(0,0, *m_onPixmap);
    }
    else
    {
        if ((m_offPixmap) && (!m_offPixmap->isNull()))
            painter.drawPixmap(0,0, *m_offPixmap);
    }
}

QSize FlatToggleButton::sizeHint() const
{
    if (!m_onPixmap) return minimumSize();

    if (!m_onPixmap->isNull())
    {
        return m_onPixmap->size();
    }
    
    return minimumSize();
}
