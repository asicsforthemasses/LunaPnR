#include "techlibinfomodel.h"

using namespace GUI;

//
// TODO: implement a base or template class for a node
//       so we get max. code re-use
//

// ********************************************************************************
//    LayerInfoNode
// ********************************************************************************

LayerInfoNode::LayerInfoNode(const QString &valueName, const QString &value)
    : NodeBase(valueName, value)
{

}

// ********************************************************************************
//    LayerInfoModel
// ********************************************************************************

LayerInfoModel::LayerInfoModel()
{
}

LayerInfoModel::~LayerInfoModel()
{
}

void LayerInfoModel::setLayer(const ChipDB::LayerInfo *layer)
{
    beginResetModel();
    m_rootNode.reset(new LayerInfoNode("Cell", ""));
    endResetModel();
}
