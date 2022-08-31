// SPDX-FileCopyrightText: 2021-2022 Niels Moseley <asicsforthemasses@gmail.com>
//
// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <QPushButton>
#include <QColor>

namespace GUI
{

class SelectColorButton : public QPushButton
{
    Q_OBJECT
public:
    SelectColorButton(QWidget *parent = nullptr);

    void setColor(const QColor &color);
    QColor getColor() const;

public slots:
    void changeColor();

signals:
    void onColorChanged();

protected:
    void paintEvent(QPaintEvent *event);

private:
    QColor m_color;
};

};