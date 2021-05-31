#pragma once

#include <memory>
#include <QWidget>
#include <QListView>
#include <QTableView>
#include <QHBoxLayout>
#include "../models/celllibmodel.h"
#include "../models/cellpinmodel.h"
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

    CellLibTableModel* getCellModel()
    {
        return m_cellModel.get();
    }

    CellPinListModel* getPinModel()
    {
        return m_pinModel.get();
    }

public slots:
    void onSelectionChanged(const QItemSelection &cur, const QItemSelection &prev);

protected:
    std::unique_ptr<CellLibTableModel> m_cellModel;
    std::unique_ptr<CellPinListModel> m_pinModel;
    
    QHBoxLayout     *m_layout;
    QTableView      *m_cellTableView;
    CellLayoutView  *m_cellLayoutView;
    QListView       *m_pinListView;
};

};  // namespace