// SPDX-FileCopyrightText: 2021-2024 Niels Moseley <asicsforthemasses@gmail.com>
//
// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <vector>
#include <QPaintEvent>
#include <QAbstractButton>

namespace GUI
{

class FlatImageButton : public QAbstractButton
{
    Q_OBJECT
public:
    FlatImageButton(QWidget *parent = nullptr);
    FlatImageButton(const QString &pixmapUrl, QWidget *parent = nullptr);

    void setPixmap(const QString &pixmapUrl);

    QSize sizeHint() const;

protected:
    void paintEvent(QPaintEvent* e) override;

    std::unique_ptr<QPixmap> m_pixmap;
};

};
