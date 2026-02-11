// SPDX-FileCopyrightText: 2021-2026 Niels Moseley <asicsforthemasses@gmail.com>
//
// SPDX-License-Identifier: GPL-3.0-only

#include "svgicon.hpp"

#include <QFile>
#include <QPainter>
#include <QPixmap>
#include <QSvgRenderer>
#include <QIconEngine>
#include <QPixmapCache>
#include <QGuiApplication>
#include <QPalette>

namespace GUI::SVGIcon
{

class IconEngine : public QIconEngine
{
public:
    IconEngine(const QString &filename) : m_filename(filename) {}

    ~IconEngine() override = default;

    [[nodiscard]] IconEngine* clone() const override
    {
        return new IconEngine(m_filename);
    }

    void paint(QPainter *painter, const QRect &rect,
        const QIcon::Mode mode,
        const QIcon::State state) override
    {
        auto pix = GUI::SVGIcon::pixmap(m_filename, rect.size(), mode, state);
        painter->drawPixmap(rect.topLeft(), pix);
    }

    QPixmap pixmap(const QSize &size, const QIcon::Mode mode, const QIcon::State state) override
    {
        return GUI::SVGIcon::pixmap(m_filename, size, mode, state);
    }

protected:
    QString m_filename;
};

QPixmap pixmap(const QString &filename,
    const QSize &sz,
    const QIcon::Mode mode,
    const QIcon::State state)
{
    Q_UNUSED (state)

    QColor color;
    switch (mode)
    {
        case QIcon::Disabled:
            color = QGuiApplication::palette().color(QPalette::Disabled, QPalette::WindowText);
            break;
        case QIcon::Selected:
            color = QGuiApplication::palette().highlightedText().color();
            break;
        default:    // intential fall-through
        case QIcon::Normal:
            color = QGuiApplication::palette().windowText().color();
            break;
    }

    QString key = filename + "-" + QString::number(sz.width())
        + "-" + QString::number(sz.height())
        + "-" + color.name();

    QPixmap pix;

    if (!QPixmapCache::find(key, &pix))
    {
        pix = QPixmap(sz);
        pix.fill(Qt::transparent);
        if (!filename.isEmpty())
        {
            QSvgRenderer renderer;
            QFile f(filename);
            QByteArray bytes;
            if (f.open(QIODevice::ReadOnly))
            {
                bytes = f.readAll();
                //bytes.replace("#000", color.name().toLatin1());
            }
            renderer.load(bytes);
            QPainter p(&pix);
            renderer.render(&p, QRect(0,0, sz.width(), sz.height()));
        }
        QPixmapCache::insert(key, pix);
    }
    return pix;
}

QIcon icon(const QString &filename)
{
    return QIcon(new IconEngine(filename));
}

};