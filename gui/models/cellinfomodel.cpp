// SPDX-FileCopyrightText: 2021-2023 Niels Moseley <asicsforthemasses@gmail.com>
//
// SPDX-License-Identifier: GPL-3.0-only

#include <array>
#include <QString>
#include <QColor>
#include <QVariant>
#include "cellinfomodel.h"

using namespace GUI;

// ********************************************************************************
//    CellInfoNode
// ********************************************************************************

CellInfoNode::CellInfoNode(const QString &valueName, QVariant value, QColor bkColor)
    : NodeBase(valueName, value)
{
    setBackgroundColor(0, bkColor);
    setBackgroundColor(1, bkColor);
}

CellInfoNode::~CellInfoNode()
{

}

// ********************************************************************************
//    CellInfoModel
// ********************************************************************************

CellInfoModel::CellInfoModel()
{
    // light blue e6ffff
    // blue  c7ffff
    // purple eabfff
    // light purple f4deff
    // light yellow ffffde
    // yellow ffffbf
    // green bfffbf
    // light green deffde
    // grey b8b8b8

    m_altColors.setColors(QColor("#D0D0D0"),QColor("#F0F0F0"));

    m_pinColor = QColor("#ffffbf");         // yellow
    m_separatorColor = QColor("#b8b8b8");   // grey
}

CellInfoModel::~CellInfoModel()
{

}

void CellInfoModel::setCell(const std::shared_ptr<ChipDB::Cell> cell)
{
    beginResetModel();
    m_rootNode.reset(new CellInfoNode("Cell", ""));

    m_altColors.resetState();

    // generic cell information
    auto areaNode = new CellInfoNode("Area (um²)", cell->m_area, m_altColors.getColorAndUpdate());
    m_rootNode->addChild(areaNode);

    auto classNode = new CellInfoNode("Class", QString::fromStdString(toString(cell->m_class)), m_altColors.getColorAndUpdate());
    m_rootNode->addChild(classNode);

    auto subclassNode = new CellInfoNode("Subclass", QString::fromStdString(toString(cell->m_subclass)), m_altColors.getColorAndUpdate());
    m_rootNode->addChild(subclassNode);

    auto siteNode = new CellInfoNode("Site", QString::fromStdString(cell->m_site), m_altColors.getColorAndUpdate());
    m_rootNode->addChild(siteNode);

    auto leakPowerNode = new CellInfoNode("Leakage pwr (nW)", cell->m_leakagePower * 1e9f, m_altColors.getColorAndUpdate());
    m_rootNode->addChild(leakPowerNode);

    auto offsetNode = new CellInfoNode("Offset ", QString::asprintf("%ld,%ld", cell->m_offset.m_x, cell->m_offset.m_y), 
        m_altColors.getColorAndUpdate());

    m_rootNode->addChild(offsetNode);

    auto separatorNode = new CellInfoNode("", "", m_separatorColor);
    m_rootNode->addChild(separatorNode);

    // expose all the pins
    bool colorSelect = false;
    for(auto pin : cell->m_pins)
    {
        m_altColors.resetState();

        auto pinNode = new CellInfoNode("Pin", QString::fromStdString(pin->name()), m_pinColor);
        pinNode->setIcon(QPixmap("://pinicon.png"));
        m_rootNode->addChild(pinNode);

        // add input/output for each pin
        int colSelect = 0;
        
        auto pinTypeStr = QString::fromStdString(toString(pin->m_iotype));
        if (pin->m_clock)
        {
            pinTypeStr += " CLOCK";
        }

        pinNode->addChild(new CellInfoNode("Type", pinTypeStr, m_altColors.getColorAndUpdate()));

        if (!pin->m_function.empty())
        {
            pinNode->addChild(new CellInfoNode("Function", QString::fromStdString(pin->m_function), m_altColors.getColorAndUpdate()));
        }

        if (pin->m_cap > 0.0)
        {
            pinNode->addChild(new CellInfoNode("Capacitance (pF)", pin->m_cap * 1.e12f, m_altColors.getColorAndUpdate()));
        }
    }

    endResetModel();
}
