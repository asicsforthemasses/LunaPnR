/*

    FlowManager Widget Source Code

    SPDX-License-Identifier: GPL-3.0-only
    SPDX-FileCopyrightText: 2022 Niels Moseley <asicsforthemasses@gmail.com>

*/

#pragma once 

#include <vector>
#include <QPaintEvent>
#include <QAbstractButton>

namespace GUI 
{

class FlatImage : public QWidget
{
    Q_OBJECT
public:
    FlatImage(QWidget *parent = nullptr);
    FlatImage(const QString &pixmapUrl, QWidget *parent = nullptr);

    void setPixmap(const QString &pixmapUrl);

    QSize sizeHint() const;

protected:
    void paintEvent(QPaintEvent* e) override;

    std::unique_ptr<QPixmap> m_pixmap;
};

};
