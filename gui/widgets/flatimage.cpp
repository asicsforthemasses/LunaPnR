// SPDX-FileCopyrightText: 2021-2025 Niels Moseley <asicsforthemasses@gmail.com>
//
// SPDX-License-Identifier: GPL-3.0-only

#include <QStyleOption>
#include <QPainter>
#include "flatimage.h"

using namespace GUI;

FlatImage::FlatImage(const QString &pixmapUrl, QWidget *parent) : QWidget(parent)
{
    setPixmap(pixmapUrl);
}

FlatImage::FlatImage(QWidget *parent)
    : QWidget(parent)
{
    setMinimumSize(QSize(24,24));
}

void FlatImage::setPixmap(const QString &pixmapUrl)
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

void FlatImage::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);

    if ((m_pixmap) && (!m_pixmap->isNull()))
        painter.drawPixmap(0,0, *m_pixmap);
}

QSize FlatImage::sizeHint() const
{
    if (!m_pixmap) return minimumSize();

    if (!m_pixmap->isNull())
    {
        return m_pixmap->size();
    }

    return minimumSize();
}
