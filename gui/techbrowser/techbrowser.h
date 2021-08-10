#pragma once

#include <memory>
#include <QWidget>
#include <QListView>
#include <QTableView>
#include <QTreeView>
#include <QGroupBox>
#include <QFrame>
#include <QHBoxLayout>
#include "../models/techlibinfomodel.h"

namespace GUI
{

class TechBrowser : public QWidget
{
    Q_OBJECT

public:
    explicit TechBrowser(QWidget *parent = nullptr);
    virtual ~TechBrowser();

    void setTechLib(ChipDB::TechLib *techLib);
    
    QSize sizeHint() const;


public slots:
    //void onCellSelectionChanged(const QItemSelection &cur, const QItemSelection &prev);

protected:
    std::unique_ptr<LayerInfoModel> m_layerInfoModel;
    //std::unique_ptr<TechInfoModel>     m_techInfoModel;
    
    QHBoxLayout     *m_layout;
    //QVBoxLayout     *m_layout2;
    //QTableView      *m_cellTableView;
    //CellLayoutView  *m_cellLayoutView;

    QTreeView       *m_layerTreeView;
};

};  // namespace