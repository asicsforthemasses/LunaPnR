// SPDX-FileCopyrightText: 2021-2025 Niels Moseley <asicsforthemasses@gmail.com>
//
// SPDX-License-Identifier: GPL-3.0-only

#include <QStyleOption>
#include <QPainter>
#include "flatimagebutton.h"

using namespace GUI;

FlatImageButton::FlatImageButton(const QString &pixmapUrl, QWidget *parent)
{
    setPixmap(pixmapUrl);
}

FlatImageButton::FlatImageButton(QWidget *parent)
    : QAbstractButton(parent)
{
    setMinimumSize(QSize(24,24));
}

void FlatImageButton::setPixmap(const QString &pixmapUrl)
{
    m_pixmap = std::make_unique<QPixmap>(pixmapUrl);
    if (m_pixmap && (!m_pixmap->isNull()))
    {
        setMinimumSize(m_pixmap->size());
    }
    else
    {
        setMinimumSize(QSize(24,24));
    }
}

void FlatImageButton::paintEvent(QPaintEvent *event)
{
    // make sure we have at least a little stylesheet support..
    QStyleOption opt;
    opt.initFrom(this);

    QPainter painter(this);
    style()->drawPrimitive(QStyle::PE_Widget, &opt, &painter, this);

    if ((m_pixmap) && (!m_pixmap->isNull()))
        painter.drawPixmap(0,0, *m_pixmap);
}

QSize FlatImageButton::sizeHint() const
{
    if (!m_pixmap) return minimumSize();

    if (!m_pixmap->isNull())
    {
        return m_pixmap->size();
    }

    return minimumSize();
}
