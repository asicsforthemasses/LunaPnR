#pragma once

#include <memory>
#include <optional>
#include <QWidget>
#include <QListView>
#include <QTableView>
#include <QTreeView>
#include <QGroupBox>
#include <QFrame>
#include <QHBoxLayout>

#include "../common/layerrenderinfo.h"
#include "../models/celllibmodel.h"
#include "../models/cellinfomodel.h"
#include "../models/layermodel.h"
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

    void setDatabase(Database *db);
    void refreshDatabase();

    QSize sizeHint() const;

public slots:
    void onCellSelectionChanged(const QItemSelection &cur, const QItemSelection &prev);

protected:
    std::unique_ptr<CellLibTableModel>  m_cellModel;
    std::unique_ptr<CellInfoModel>      m_cellInfoModel;
    std::unique_ptr<LayerListModel>     m_layerListModel;

    QHBoxLayout     *m_layout;
    QVBoxLayout     *m_layout2;
    QTableView      *m_cellTableView;
    CellLayoutView  *m_cellLayoutView;

    QTreeView       *m_cellTreeView;
    QListView       *m_layerView;
    
    Database        *m_db;
};

};  // namespace