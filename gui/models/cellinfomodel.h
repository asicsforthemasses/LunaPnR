/*
  LunaPnR Source Code
  
  SPDX-License-Identifier: GPL-3.0-only
  SPDX-FileCopyrightText: 2022 Niels Moseley <asicsforthemasses@gmail.com>
*/

#pragma once

#include <QAbstractItemModel>
#include <QString>

#include <string>
#include <vector>

#include "lunacore.h"
#include "../common/guihelpers.h"
#include "modelnodebase.h"

namespace GUI
{

class CellInfoNode : public NodeBase
{
public:
    CellInfoNode(const QString &valueName, QVariant value, QColor bkColor = Qt::white);
    virtual ~CellInfoNode();
};

class CellInfoModel : public ModelBase
{
public:
    CellInfoModel();
    virtual ~CellInfoModel();

    void setCell(const std::shared_ptr<ChipDB::Cell> cell);

protected:
    QColor m_pinColor;
    QColor m_separatorColor;
    AlternatingColors m_altColors;
};

};
