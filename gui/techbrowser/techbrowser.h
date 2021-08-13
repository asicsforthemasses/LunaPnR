#pragma once

#include <memory>
#include <QWidget>
#include <QListView>
#include <QTableView>
#include <QTreeView>
#include <QGroupBox>
#include <QFrame>
#include <QHBoxLayout>
#include "../common/layerrenderinfo.h"
#include "../models/techlibinfomodel.h"
#include "../widgets/colorbutton.h"

namespace GUI
{

class TechBrowser : public QWidget
{
    Q_OBJECT

public:
    explicit TechBrowser(QWidget *parent = nullptr);
    virtual ~TechBrowser();

    void setTechLib(ChipDB::TechLib *techLib);
    void setLayerRenderInfo(LayerRenderInfoDB *renderInfoDB);
    
    QSize sizeHint() const;


public slots:
    void onLayerSelectionChanged(const QItemSelection &cur, const QItemSelection &prev);
    void onLayerColorChanged();

protected:
    std::unique_ptr<LayerInfoModel>     m_layerInfoModel;
    std::unique_ptr<LayerTableModel>    m_layerTableModel;
    
    QHBoxLayout     *m_layout;
    //QVBoxLayout     *m_layout2;
    QTableView      *m_layerTableView;
    //CellLayoutView  *m_cellLayoutView;

    QTreeView           *m_layerTreeView;
    SelectColorButton   *m_colorButton;

    LayerRenderInfoDB   *m_layerRenderInfoDB;
};

};  // namespace