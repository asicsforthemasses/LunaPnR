#pragma once

#include <memory>
#include <QWidget>
#include <QListView>
#include <QTableView>
#include <QTreeView>
#include <QGroupBox>
#include <QFrame>
#include <QHBoxLayout>
#include "../models/celllibmodel.h"
#include "../models/cellinfomodel.h"
#include "../propertyview/propertyview.h"
#include "celllayoutview.h"

namespace GUI
{

class CellBrowser : public QWidget
{
    Q_OBJECT

public:
    explicit CellBrowser(QWidget *parent = nullptr);
    virtual ~CellBrowser();

    void setCellLib(ChipDB::CellLib *cellLib);
    
    QSize sizeHint() const;

#if 0
    CellLibTableModel* getCellModel()
    {
        return m_cellModel.get();
    }

    CellPinListModel* getPinModel()
    {
        return m_pinModel.get();
    }
#endif

public slots:
    void onCellSelectionChanged(const QItemSelection &cur, const QItemSelection &prev);
    void onPinSelectionChanged(const QItemSelection &cur, const QItemSelection &prev);

protected:
    std::unique_ptr<CellLibTableModel> m_cellModel;
    std::unique_ptr<CellInfoModel>     m_cellInfoModel;
    //std::unique_ptr<PinInfoTableModel> m_pinInfoModel;
    
    QHBoxLayout     *m_layout;
    QVBoxLayout     *m_layout2;
    QFrame          *m_pinGroup;
    QTableView      *m_cellTableView;
    CellLayoutView  *m_cellLayoutView;

    QTreeView       *m_cellTreeView;

    //QListView       *m_pinListView;
    //QTableView      *m_pinInfoTable;
    //PropertyView    *m_pinInfoView;
};

};  // namespace