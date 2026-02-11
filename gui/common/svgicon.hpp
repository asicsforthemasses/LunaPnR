// SPDX-FileCopyrightText: 2021-2026 Niels Moseley <asicsforthemasses@gmail.com>
//
// SPDX-License-Identifier: GPL-3.0-only

#pragma once
#include <QIcon>
#include <QString>

namespace GUI::SVGIcon 
{
    /** Create a QIcon from an SVG file.
        This function supports the Qt resource
        system.
    */
    QIcon icon(const QString &filename);

    /** create a QPixmap from an SVG file.
        This function supports the Qt resource
        system.    
    */
    QPixmap pixmap(const QString &filename,
        const QSize &sz,
        const QIcon::Mode mode,
        const QIcon::State state);
};
