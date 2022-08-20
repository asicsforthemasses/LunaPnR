#include <cassert>
#include <algorithm>

#include <QHeaderView>
#include <QMessageBox>

#include "projectmanager.h"
#include "../common/customevents.h"
#include "../widgets/blockcontainer.h"
#include "../widgets/flatimagebutton.h"
#include "../widgets/flatactiontile.h"
#include "../widgets/blockframe.h"
#include "../projectmanager/filesetupmanager.h"
#include "../floorplandialog/floorplandialog.h"

using namespace GUI;

ProjectManager::ProjectManager(Database &db, QWidget *parent) : QWidget(parent), m_db(db), m_projectSetup(db.m_projectSetup)
{
    create();
}

void ProjectManager::create()
{
    m_managerLayout = new QVBoxLayout();

    // ******************************************************************************************
    // BLOCK 1
    // ******************************************************************************************
    auto block = new GUI::BlockContainer();
    block->setBlockName("Design Setup");
    block->header()->setStyleSheet("background-color: #e8e7e8; ");

    auto buttonForHeader = new GUI::FlatImageButton("://images/remove.png");
    block->header()->addWidget(buttonForHeader);
    buttonForHeader = new GUI::FlatImageButton("://images/add.png");
    block->header()->addWidget(buttonForHeader);

    m_fileSetupManager = new GUI::FileSetupManager();
    m_fileSetupManager->header()->hide();
    m_fileSetupManager->addCategory("LEF", ".lef", &m_projectSetup.m_lefFiles);
    m_fileSetupManager->addCategory("LIB", ".lib", &m_projectSetup.m_libFiles);
    m_fileSetupManager->addCategory("Verilog", ".v", &m_projectSetup.m_verilogFiles);    
    m_fileSetupManager->addCategory("Timing constraints", ".sdc", &m_projectSetup.m_timingConstraintFiles);
    m_fileSetupManager->addCategory("Layers", ".layers", &m_projectSetup.m_layerFiles);

    block->addWidget(m_fileSetupManager,1);
    m_managerLayout->addWidget(block);

    // ******************************************************************************************
    // BLOCK 2
    // ******************************************************************************************
    block = new GUI::BlockContainer();
    block->setBlockName("Initial Placement");
    block->header()->setStyleSheet("background-color: #e8e7e8; ");

    auto blockFrame = new GUI::BlockFrame();
    
    auto actionTile = new GUI::FlatActionTile("Floorplan setup", "://images/floorplan.png", "://images/properties.png", "FLOORPLANSETUP");
    connect(actionTile, &GUI::FlatActionTile::onAction, this, &ProjectManager::onFloorplanSetup);
    m_tiles.push_back(actionTile);
    blockFrame->addWidget(actionTile);

    auto statusTile = new GUI::FlatStatusTile("Pre-place checks", "://images/floorplan.png", "PreflightChecks");
    m_tiles.push_back(statusTile);
    blockFrame->addWidget(statusTile);

    actionTile = new GUI::FlatActionTile("Place", "://images/floorplan.png", "://images/go.png", "PLACE");
    connect(actionTile, &GUI::FlatActionTile::onAction, this, &ProjectManager::onAction);
    m_tiles.push_back(actionTile);
    blockFrame->addWidget(actionTile);

    block->addWidget(blockFrame);

    m_managerLayout->addWidget(block);

    // ******************************************************************************************
    // BLOCK 3
    // ******************************************************************************************
    block = new GUI::BlockContainer();
    block->setBlockName("Clock tree synthesis");
    block->header()->setStyleSheet("background-color: #e8e7e8; ");

    blockFrame = new GUI::BlockFrame();
    
    actionTile = new GUI::FlatActionTile("CTS setup", "://images/floorplan.png", "://images/properties.png", "CTSSETUP");
    connect(actionTile, &GUI::FlatActionTile::onAction, this, &ProjectManager::onAction);
    m_tiles.push_back(actionTile);
    blockFrame->addWidget(actionTile);

    actionTile = new GUI::FlatActionTile("Create tree", "://images/floorplan.png", "://images/go.png", "CREATECTSTREE");
    connect(actionTile, &GUI::FlatActionTile::onAction, this, &ProjectManager::onAction);
    m_tiles.push_back(actionTile);
    blockFrame->addWidget(actionTile);

    actionTile = new GUI::FlatActionTile("Timing Report", "://images/floorplan.png", "://images/go.png", "TIMINGREPORT1");
    connect(actionTile, &GUI::FlatActionTile::onAction, this, &ProjectManager::onAction);
    m_tiles.push_back(actionTile);
    blockFrame->addWidget(actionTile);

    block->addWidget(blockFrame);

    m_managerLayout->addWidget(block);

    // ******************************************************************************************
    // BLOCK 4
    // ******************************************************************************************
    block = new GUI::BlockContainer();
    block->setBlockName("Global routing");
    block->header()->setStyleSheet("background-color: #e8e7e8; ");

    blockFrame = new GUI::BlockFrame();
    
    //actionTile = new GUI::FlatActionTile("CTS setup", "://images/floorplan.png", "://images/properties.png");
    //connect(actionTile, &GUI::FlatActionTile::onAction, this, &ProjectManager::onCTSSetup);
    //blockFrame->addWidget(actionTile);

    //actionTile = new GUI::FlatActionTile("Create tree", "://images/floorplan.png", "://images/go.png");
    //blockFrame->addWidget(actionTile);

    block->addWidget(blockFrame);

    m_managerLayout->addWidget(block);

    // ******************************************************************************************
    // BLOCK 5
    // ******************************************************************************************
    block = new GUI::BlockContainer();
    block->setBlockName("Detail routing");
    block->header()->setStyleSheet("background-color: #e8e7e8; ");

    blockFrame = new GUI::BlockFrame();
    
    //actionTile = new GUI::FlatActionTile("CTS setup", "://images/floorplan.png", "://images/properties.png");
    //connect(actionTile, &GUI::FlatActionTile::onAction, this, &ProjectManager::onCTSSetup);
    //blockFrame->addWidget(actionTile);

    //actionTile = new GUI::FlatActionTile("Create tree", "://images/floorplan.png", "://images/go.png");
    //blockFrame->addWidget(actionTile);

    block->addWidget(blockFrame);

    m_managerLayout->addWidget(block);

    // ******************************************************************************************
    // BLOCK 6
    // ******************************************************************************************
    block = new GUI::BlockContainer();
    block->setBlockName("Check design");
    block->header()->setStyleSheet("background-color: #e8e7e8; ");

    blockFrame = new GUI::BlockFrame();
    
    //actionTile = new GUI::FlatActionTile("CTS setup", "://images/floorplan.png", "://images/properties.png");
    //connect(actionTile, &GUI::FlatActionTile::onAction, this, &ProjectManager::onCTSSetup);
    //blockFrame->addWidget(actionTile);

    actionTile = new GUI::FlatActionTile("Timing Report", "://images/floorplan.png", "://images/go.png", "TIMINGREPORT2");
    connect(actionTile, &GUI::FlatActionTile::onAction, this, &ProjectManager::onAction);
    m_tiles.push_back(actionTile);
    blockFrame->addWidget(actionTile);

    actionTile = new GUI::FlatActionTile("DRC", "://images/floorplan.png", "://images/go.png", "DRC");
    connect(actionTile, &GUI::FlatActionTile::onAction, this, &ProjectManager::onAction);
    m_tiles.push_back(actionTile);
    blockFrame->addWidget(actionTile);

    //actionTile = new GUI::FlatActionTile("Create tree", "://images/floorplan.png", "://images/go.png");
    //blockFrame->addWidget(actionTile);

    block->addWidget(blockFrame);

    m_managerLayout->addWidget(block);

    // ******************************************************************************************
    // BLOCK 7
    // ******************************************************************************************
    block = new GUI::BlockContainer();
    block->setBlockName("Tape out");
    block->header()->setStyleSheet("background-color: #e8e7e8; ");

    blockFrame = new GUI::BlockFrame();
    
    actionTile = new GUI::FlatActionTile("Write GDS2", "://images/floorplan.png", "://images/go.png", "WRITEGDS2");
    connect(actionTile, &GUI::FlatActionTile::onAction, this, &ProjectManager::onAction);
    m_tiles.push_back(actionTile);
    blockFrame->addWidget(actionTile);

    actionTile = new GUI::FlatActionTile("Write DEF", "://images/floorplan.png", "://images/go.png", "WRITEDEF");
    connect(actionTile, &GUI::FlatActionTile::onAction, this, &ProjectManager::onAction);
    m_tiles.push_back(actionTile);
    blockFrame->addWidget(actionTile);

    block->addWidget(blockFrame);

    m_managerLayout->addWidget(block);
    m_managerLayout->addStretch(1);

    setLayout(m_managerLayout);
}

void ProjectManager::repopulate()
{
    if (m_fileSetupManager != nullptr)
    {
        m_fileSetupManager->repopulate();
    }
}

bool ProjectManager::event(QEvent * event)
{
    if (event->type() == GUI::TaskListEvent::EventType)
    {
        auto e = static_cast<GUI::TaskListEvent*>(event);

        // search through action tiles and set the status accordingly
        const QString actionName = e->name();

        auto iter = std::find_if(m_tiles.begin(), m_tiles.end(), [&actionName]
            (const FlatTileBase *tile)
            {
                return tile->actionName() == actionName;
            }
        );

        if (iter == m_tiles.end())
        {
            std::cout << "ProjectManager event: tile with action name " << actionName.toStdString() << " not found!\n";
        }
        else
        {
            //std::cout << "ProjectManager event: tile " << (*iter)->actionTitle().toStdString() << " with action name " << actionName.toStdString() << " update!\n";
            switch(e->status())
            {
            case Tasks::Task::Status::RESET:
                (*iter)->setStatus(GUI::FlatActionTile::Status::NONE);
                break;
            case Tasks::Task::Status::DONE_OK:
                (*iter)->setStatus(GUI::FlatActionTile::Status::OK);
                break;
            case Tasks::Task::Status::DONE_ERROR:
                (*iter)->setStatus(GUI::FlatActionTile::Status::ERROR);
                break;
            case Tasks::Task::Status::RUNNING:
                (*iter)->setStatus(GUI::FlatActionTile::Status::RUNNING);
                break;
            default:
                //FIXME: handle progress!
                break;
            }
        }

        return true;
    }
    
    return QWidget::event(event);
}

void ProjectManager::onFloorplanSetup(QString actionName)
{
    FloorplanDialog dialog(m_db);
    int result = dialog.exec();
    if (result == QDialog::Accepted)
    {
        std::cout << "FloorplanDialog was excepted\n";
    }
    else
    {
        std::cout << "FloorplanDialog was rejected\n";
    }
}

