#include <array>
#include <QString>
#include <QColor>
#include <QVariant>
#include "moduleinfomodel.h"

using namespace GUI;

// ********************************************************************************
//    ModuleInfoNode
// ********************************************************************************

ModuleInfoNode::ModuleInfoNode(const QString &valueName, QVariant value, QColor bkColor)
    : NodeBase(valueName, value)
{
    setBackgroundColor(0, bkColor);
    setBackgroundColor(1, bkColor);
}

ModuleInfoNode::~ModuleInfoNode()
{
}

// ********************************************************************************
//    ModuleInfoModel
// ********************************************************************************

ModuleInfoModel::ModuleInfoModel()
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

ModuleInfoModel::~ModuleInfoModel()
{

}

void ModuleInfoModel::setModule(const ChipDB::Module *module)
{
    beginResetModel();
    m_rootNode.reset(new ModuleInfoNode("Module", ""));

    m_altColors.resetState();

    // generic cell information
    int32_t numberOfInstances = module->m_netlist.m_instances.size();
    auto instancesNode = new ModuleInfoNode("Instances #", numberOfInstances, m_altColors.getColorAndUpdate());
    m_rootNode->addChild(instancesNode);

    int32_t numberOfPins = module->m_pins.size();
    auto pinsNode = new ModuleInfoNode("Pins #", numberOfPins, m_altColors.getColorAndUpdate());
    m_rootNode->addChild(pinsNode);

#if 0
    auto classNode = new ModuleInfoNode("Class", QString::fromStdString(toString(cell->m_class)), m_altColors.getColorAndUpdate());
    m_rootNode->addChild(classNode);

    auto subclassNode = new ModuleInfoNode("Subclass", QString::fromStdString(toString(cell->m_subclass)), m_altColors.getColorAndUpdate());
    m_rootNode->addChild(subclassNode);

    auto siteNode = new ModuleInfoNode("Site", QString::fromStdString(cell->m_site), m_altColors.getColorAndUpdate());
    m_rootNode->addChild(siteNode);

    auto leakPowerNode = new ModuleInfoNode("Leakage pwr (nW)", cell->m_leakagePower * 1e9f, m_altColors.getColorAndUpdate());
    m_rootNode->addChild(leakPowerNode);

    auto offsetNode = new ModuleInfoNode("Offset ", QString::asprintf("%ld,%ld", cell->m_offset.m_x, cell->m_offset.m_y), 
        m_altColors.getColorAndUpdate());

    m_rootNode->addChild(offsetNode);

#endif

    auto separatorNode = new ModuleInfoNode("", "", m_separatorColor);
    m_rootNode->addChild(separatorNode);

    // expose all the pins
    bool colorSelect = false;
    for(auto const& pin : module->m_pins)
    {
        m_altColors.resetState();

        auto pinNode = new ModuleInfoNode("Pin", QString::fromStdString(pin.m_name), m_pinColor);
        pinNode->setIcon(QPixmap("://pinicon.png"));
        m_rootNode->addChild(pinNode);

        // add input/output for each pin
        int colSelect = 0;
        
        auto pinTypeStr = QString::fromStdString(toString(pin.m_iotype));
        if (pin.m_clock)
        {
            pinTypeStr += " CLOCK";
        }

        pinNode->addChild(new ModuleInfoNode("Type", pinTypeStr, m_altColors.getColorAndUpdate()));

        if (!pin.m_function.empty())
        {
            pinNode->addChild(new ModuleInfoNode("Function", QString::fromStdString(pin.m_function), m_altColors.getColorAndUpdate()));
        }

        if (pin.m_cap > 0.0)
        {
            pinNode->addChild(new ModuleInfoNode("Capacitance (pF)", pin.m_cap * 1.e12f, m_altColors.getColorAndUpdate()));
        }
        
    }

    endResetModel();
}
