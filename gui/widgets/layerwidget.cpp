// SPDX-FileCopyrightText: 2021-2024 Niels Moseley <asicsforthemasses@gmail.com>
//
// SPDX-License-Identifier: GPL-3.0-only

#include "layerwidget.h"

namespace GUI
{

LayerWidget::LayerWidget(QWidget *parent) : QTableView(parent)
{
    setSelectionBehavior(QTableView::SelectRows);
    setSelectionMode(QAbstractItemView::SingleSelection);
}

};
