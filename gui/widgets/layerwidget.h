// SPDX-FileCopyrightText: 2021-2024 Niels Moseley <asicsforthemasses@gmail.com>
//
// SPDX-License-Identifier: GPL-3.0-only

#pragma once
#include "lunacore.h"

#include <QTableView>

/** List of layers with a specific ordering.
    layers can be enabled/disabled by the user
*/
namespace GUI
{

class LayerWidget : public QTableView
{
public:
    LayerWidget(QWidget *parent = nullptr);

};

};