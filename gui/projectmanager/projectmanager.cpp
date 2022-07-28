#include <cassert>
#include <QHeaderView>
#include <QMessageBox>

#include "projectmanager.h"
#include "../widgets/blockcontainer.h"
#include "../widgets/flatimagebutton.h"
#include "../widgets/flatactiontile.h"
#include "../widgets/blockframe.h"
#include "../projectmanager/filesetupmanager.h"

using namespace GUI;

ProjectManager::ProjectManager(ProjectSetup *projectSetup, QWidget *parent) : QWidget(parent), m_projectSetup(projectSetup)
{
    assert(projectSetup != nullptr);
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
    m_fileSetupManager->addCategory("LEF", ".lef", &m_projectSetup->m_lefFiles);
    m_fileSetupManager->addCategory("LIB", ".lib", &m_projectSetup->m_libFiles);
    m_fileSetupManager->addCategory("Verilog", ".v", &m_projectSetup->m_verilogFiles);
    //m_projectmanager->addCategory("Layers", ".layers", &m_projectSetup.m_layerFiles);
    m_fileSetupManager->addCategory("Timing constraints", ".sdc", &m_projectSetup->m_timingConstraintFiles);

    block->addWidget(m_fileSetupManager,1);
    m_managerLayout->addWidget(block);

    // ******************************************************************************************
    // BLOCK 2
    // ******************************************************************************************
    block = new GUI::BlockContainer();
    block->setBlockName("Initial Placement");
    block->header()->setStyleSheet("background-color: #e8e7e8; ");

    auto blockFrame = new GUI::BlockFrame();
    
    auto actionTile = new GUI::FlatActionTile("Floorplan setup", "://images/floorplan.png", "://images/properties.png");
    connect(actionTile, &GUI::FlatActionTile::onAction, this, &ProjectManager::onFloorplanSetup);
    blockFrame->addWidget(actionTile);

    actionTile = new GUI::FlatActionTile("Place", "://images/floorplan.png", "://images/go.png");
    connect(actionTile, &GUI::FlatActionTile::onAction, this, &ProjectManager::onPlace);
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
    
    actionTile = new GUI::FlatActionTile("CTS setup", "://images/floorplan.png", "://images/properties.png");
    connect(actionTile, &GUI::FlatActionTile::onAction, this, &ProjectManager::onCTSSetup);
    blockFrame->addWidget(actionTile);

    actionTile = new GUI::FlatActionTile("Create tree", "://images/floorplan.png", "://images/go.png");
    blockFrame->addWidget(actionTile);

    actionTile = new GUI::FlatActionTile("Timing Report", "://images/floorplan.png", "://images/go.png");
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

    actionTile = new GUI::FlatActionTile("Timing Report", "://images/floorplan.png", "://images/go.png");
    blockFrame->addWidget(actionTile);

    actionTile = new GUI::FlatActionTile("DRC", "://images/floorplan.png", "://images/go.png");
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
    
    //actionTile = new GUI::FlatActionTile("CTS setup", "://images/floorplan.png", "://images/properties.png");
    //connect(actionTile, &GUI::FlatActionTile::onAction, this, &ProjectManager::onCTSSetup);
    //blockFrame->addWidget(actionTile);

    actionTile = new GUI::FlatActionTile("Create GDS2", "://images/floorplan.png", "://images/go.png");
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

void ProjectManager::onFloorplanSetup()
{
    QMessageBox msgBox;
    msgBox.setText("Floorplanning setup clicked");
    msgBox.exec();
}

void ProjectManager::onCTSSetup()
{
    QMessageBox msgBox;
    msgBox.setText("CTS setup clicked");
    msgBox.exec();
}
