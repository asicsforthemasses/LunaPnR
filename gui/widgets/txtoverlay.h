// SPDX-FileCopyrightText: 2021-2022 Niels Moseley, <n.a.moseley@moseleyinstruments.com>, et al.
//
// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <QWidget>
#include <QPainter>

namespace GUI
{

class TxtOverlay : public QWidget
{
    Q_OBJECT

public:
    explicit TxtOverlay(QWidget *parent = nullptr) : QWidget(parent)
    {
        m_margins = QMargins(12,8,12,8);
        setAttribute(Qt::WA_NoSystemBackground);
        setAttribute(Qt::WA_TransparentForMouseEvents);
        setText("");
    }

    void setText(const QString &txt);

protected:
    
    void paintEvent(QPaintEvent *) override;

    QString     m_txt;
    QRect       m_txtRect;
    QMargins    m_margins;
};

};