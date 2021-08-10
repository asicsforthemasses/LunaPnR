#pragma once

#include <memory>
#include "lunacore.h"
#include "modelnodebase.h"

namespace GUI
{

class TechLibInfoNode : public NodeBase
{
public:
    TechLibInfoNode(const QString &valueName, const QString &value);
};

class TechLibInfoModel : public ModelBase
{
public:    
    TechLibInfoModel();
    virtual ~TechLibInfoModel();

    void setTechLib(const ChipDB::TechLib *techlib);

protected:
    //QColor m_pinColor;
    //QColor m_separatorColor;
    //AlternatingColors m_altColors;
};


class LayerInfoNode : public NodeBase
{
public:
    LayerInfoNode(const QString &valueName, const QString &value);
};


class LayerInfoModel : public ModelBase
{
public:
    LayerInfoModel();
    virtual ~LayerInfoModel();

    void setLayer(const ChipDB::LayerInfo *layer);

protected:
    //QColor m_pinColor;
    //QColor m_separatorColor;
    //AlternatingColors m_altColors;
};

};  // namespace