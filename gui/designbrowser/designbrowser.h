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

#include "../common/database.h"
#include "../models/modulemodel.h"

namespace GUI
{

class DesignBrowser : public QWidget
{
    Q_OBJECT

public:
    explicit DesignBrowser(QWidget *parent = nullptr);
    virtual ~DesignBrowser();

    void setDatabase(Database *db);
    void refreshDatabase();

    QSize sizeHint() const;

public slots:
    void onModuleSelectionChanged(const QItemSelection &cur, const QItemSelection &prev);

protected:
    std::unique_ptr<ModuleTableModel>  m_moduleModel;
    //std::unique_ptr<CellInfoModel>      m_cellInfoModel;
    
    QHBoxLayout     *m_layout;
    QVBoxLayout     *m_layout2;
    QTableView      *m_moduleTableView;
    //CellLayoutView  *m_cellLayoutView;

    //QTreeView       *m_cellTreeView;

    Database        *m_db;
};

};  // namespace