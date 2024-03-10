// SPDX-FileCopyrightText: 2021-2024 Niels Moseley <asicsforthemasses@gmail.com>
//
// SPDX-License-Identifier: GPL-3.0-only

#include "clickablelabel.h"

using namespace GUI;

ClickableLabel::ClickableLabel(QWidget* parent) : QLabel(parent)
{
}

ClickableLabel::ClickableLabel(const QString &text, QWidget* parent) : QLabel(text, parent)
{
}

void ClickableLabel::mousePressEvent(QMouseEvent* event)
{
    emit clicked();
}
