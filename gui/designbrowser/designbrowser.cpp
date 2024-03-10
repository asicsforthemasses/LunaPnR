// SPDX-FileCopyrightText: 2021-2024 Niels Moseley <asicsforthemasses@gmail.com>
//
// SPDX-License-Identifier: GPL-3.0-only


#include "designbrowser.h"
#include <QHeaderView>
#include <QLabel>

using namespace GUI;

DesignBrowser::DesignBrowser(QWidget *parent) : QWidget(parent)
{
    setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);

    // module table view
    m_moduleTableView = new QTableView(parent);
    m_moduleTableView->setSelectionBehavior(QTableView::SelectRows);
    m_moduleTableView->setSelectionMode(QAbstractItemView::SingleSelection);
    m_moduleTableView->setSizeAdjustPolicy(QAbstractScrollArea::AdjustToContents);
    m_moduleTableView->horizontalHeader()->setStretchLastSection(true);

    m_moduleModel.reset(new ModuleTableModel(nullptr));
    m_moduleTableView->setModel(m_moduleModel.get());

    // module information view
    m_moduleTreeView = new QTreeView();
    m_moduleTreeView->setEditTriggers(QAbstractItemView::NoEditTriggers); // make read-only
    m_moduleTreeView->setHeaderHidden(true);

    m_moduleInfoModel.reset(new ModuleInfoModel());
    m_moduleTreeView->setModel(m_moduleInfoModel.get());

    //m_layout2 = new QVBoxLayout();
    //m_layout2->addWidget(new QLabel("Cell information"),0);
    //m_layout2->addWidget(m_cellTreeView,1);

    m_layout = new QHBoxLayout();
    m_layout->addWidget(m_moduleTableView,1);
    m_layout->addWidget(m_moduleTreeView,2);
    //m_layout->addLayout(m_layout2,1);

    setLayout(m_layout);

    connect(m_moduleTableView->selectionModel(),
        SIGNAL(selectionChanged(const QItemSelection&, const QItemSelection&)),
        this,
        SLOT(onModuleSelectionChanged(const QItemSelection&, const QItemSelection&)));
}

DesignBrowser::~DesignBrowser()
{

}

QSize DesignBrowser::sizeHint() const
{
    return m_moduleTreeView->sizeHint();
}

void DesignBrowser::setDatabase(std::shared_ptr<Database> db)
{
    m_db = db;
    m_moduleModel->setModuleLib(db->moduleLib());
}

void DesignBrowser::refreshDatabase()
{
    setDatabase(m_db);
}

void DesignBrowser::onModuleSelectionChanged(const QItemSelection &cur, const QItemSelection &prev)
{
    QModelIndex index = m_moduleTableView->currentIndex();

    if (index.isValid())
    {
        auto module = m_moduleModel->getModule(index.row());
        if (module != nullptr)
        {
            m_moduleInfoModel->setModule(module);
            update();
            Logging::doLog(Logging::LogType::VERBOSE, "Selected module %s\n", module->name().c_str());
        }
    }
}

