// SPDX-FileCopyrightText: 2021-2022 Niels Moseley, <n.a.moseley@moseleyinstruments.com>, et al.
//
// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <QScrollArea>
#include "../common/hatchlibrary.h"

namespace GUI
{

class HatchDisplay : public QScrollArea
{
    Q_OBJECT

public:
    HatchDisplay(HatchLibrary &hatchLibrary, QWidget *parent = nullptr);

signals:
    void clicked(int hatchindex);

protected slots:
    void onHatchClick();

protected:
    HatchLibrary &m_hatchLibrary;
};

};