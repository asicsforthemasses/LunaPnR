#pragma once

#include <memory>
#include <QWidget>
#include <QListView>
#include <QTableView>
#include <QTreeView>
#include <QGroupBox>
#include <QFrame>
#include <QHBoxLayout>
#include "../common/database.h"
#include "../models/techlibinfomodel.h"
#include "../widgets/colorbutton.h"
#include "../widgets/hatchbutton.h"

namespace GUI
{

class TechBrowser : public QWidget
{
    Q_OBJECT

public:
    explicit TechBrowser(QWidget *parent = nullptr);
    virtual ~TechBrowser();

    void setDatabase(std::shared_ptr<Database> db);
    void refreshDatabase();
    
    QSize sizeHint() const;

public slots:
    void onLayerSelectionChanged(const QItemSelection &cur, const QItemSelection &prev);
    void onLayerColorChanged();
    void onChangeHatch();
    void onLayerObsColorChanged();
    void onChangeObsHatch();

protected:
    std::unique_ptr<LayerInfoModel>     m_layerInfoModel;
    std::unique_ptr<LayerTableModel>    m_layerTableModel;
    
    QHBoxLayout     *m_mainLayout;
    QTableView      *m_layerTableView;

    QTreeView           *m_layerTreeView;
    SelectColorButton   *m_colorButton;
    SelectHatchButton   *m_hatchButton;
    SelectColorButton   *m_colorObsButton;
    SelectHatchButton   *m_hatchObsButton;

    std::shared_ptr<Database> m_db;
};

};  // namespace