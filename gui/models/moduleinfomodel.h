// SPDX-FileCopyrightText: 2021-2025 Niels Moseley <asicsforthemasses@gmail.com>
//
// SPDX-License-Identifier: GPL-3.0-only

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

class ModuleInfoNode : public NodeBase
{
public:
    ModuleInfoNode(const QString &valueName, QVariant value, QColor bkColor = Qt::white);
    virtual ~ModuleInfoNode();
};

class ModuleInfoModel : public ModelBase
{
public:
    ModuleInfoModel();
    virtual ~ModuleInfoModel();

    void setModule(std::shared_ptr<ChipDB::Module> module);

protected:
    QColor m_pinColor;
    QColor m_separatorColor;
    AlternatingColors m_altColors;
};

};
