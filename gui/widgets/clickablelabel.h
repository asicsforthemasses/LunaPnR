// SPDX-FileCopyrightText: 2021-2023 Niels Moseley <asicsforthemasses@gmail.com>
//
// SPDX-License-Identifier: GPL-3.0-only

#pragma once 

#include <QLabel>
#include <QMouseEvent>

namespace GUI
{

class ClickableLabel : public QLabel
{ 
    Q_OBJECT 
public:
    explicit ClickableLabel(QWidget* parent = nullptr);
    ClickableLabel(const QString &text, QWidget* parent = nullptr);

signals:
    void clicked();

protected:
    void mousePressEvent(QMouseEvent* event);
};

};
