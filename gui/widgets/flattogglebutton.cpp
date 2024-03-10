// SPDX-FileCopyrightText: 2021-2024 Niels Moseley <asicsforthemasses@gmail.com>
//
// SPDX-License-Identifier: GPL-3.0-only

#include <QStyleOption>
#include <QStylePainter>
#include <QBrush>
#include <QPalette>
#include <QPainter>
#include "flattogglebutton.h"

using namespace GUI;

FlatToggleButton::FlatToggleButton(QWidget *parent)
    : QAbstractButton(parent)
{
    m_offPixmap = std::make_unique<QPixmap>("://images/folded.png");
    m_onPixmap  = std::make_unique<QPixmap>("://images/unfolded.png");

    if ((m_onPixmap) && (!m_onPixmap->isNull()))
    {
        const auto margins = contentsMargins();
        const auto extraWidth  = margins.left() + margins.right();
        const auto extraHeight = margins.top() + margins.bottom();
        const auto minSize = m_onPixmap->size() + QSize{extraWidth, extraHeight};
        setMinimumSize(minSize);
    }

    setCheckable(true);

    installEventFilter(this);
}

void FlatToggleButton::paintEvent(QPaintEvent *event)
{
    QStylePainter painter(this);

    QStyleOptionButton option;
    option.initFrom(this);
    option.features = QStyleOptionButton::None;
    option.features |= QStyleOptionButton::Flat;

    painter.drawControl(QStyle::CE_PushButton, option);

    if (m_hover)
    {
        auto bkBrush = palette().window();
        auto fillRect = rect().adjusted(0,0,-1,-1);

        painter.fillRect(fillRect, bkBrush.color().darker(110));
    }

    const auto margins = contentsMargins();
    bool state = isChecked();
    if (state)
    {
        if ((m_onPixmap) && (!m_onPixmap->isNull()))
        {
            auto margins = contentsMargins();
            painter.drawPixmap(margins.left(), margins.top(), *m_onPixmap);
        }
    }
    else
    {
        if ((m_offPixmap) && (!m_offPixmap->isNull()))
            painter.drawPixmap(margins.left(), margins.top(), *m_offPixmap);
    }
}

QSize FlatToggleButton::sizeHint() const
{
    return minimumSize();
}

bool FlatToggleButton::eventFilter(QObject *watched, QEvent *event)
{
    //auto button = qobject_cast<FlatToggleButton*>(watched);

    if (event->type() == QEvent::Enter)
    {
        m_hover = true;
        return true;
    }
    else if (event->type() == QEvent::Leave)
    {
        m_hover = false;
        return true;
    }

    return false;
}

void FlatToggleButton::resizeEvent(QResizeEvent *e)
{
    if ((m_onPixmap) && (!m_onPixmap->isNull()))
    {
        const auto margins = contentsMargins();
        const auto extraWidth  = margins.left() + margins.right();
        const auto extraHeight = margins.top() + margins.bottom();
        const auto minSize = m_onPixmap->size() + QSize{extraWidth, extraHeight};
        setMinimumSize(minSize);
    }
}
