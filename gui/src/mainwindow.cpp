
#include <QAction>
#include <QTimer>
#include <QTabWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QApplication>
#include <QMessageBox>
#include <QFileDialog>

#include <QJsonDocument>
#include <QFile>

#include <fstream>

#include "mainwindow.h"

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent)
{
    m_floorplanDirty = true;
    m_techLibDirty   = true;
    m_cellLibDirty   = true;

    setLogLevel(LOG_VERBOSE);

    //m_menuBar = new QMenuBar();
    //setMenuBar(m_menuBar);

    createActions();
    createMenus();
    
    // create tabs
    m_mainTabWidget = new QTabWidget(this);

    m_designBrowser = new GUI::DesignBrowser(this);
    m_designBrowser->setDatabase(&m_db);
    m_mainTabWidget->addTab(m_designBrowser, tr("Design Browser"));

    m_cellBrowser = new GUI::CellBrowser(this);
    m_cellBrowser->setDatabase(&m_db);
    m_mainTabWidget->addTab(m_cellBrowser, tr("Cell Browser"));

    m_techBrowser = new GUI::TechBrowser(this);
    m_techBrowser->setDatabase(&m_db);
    m_mainTabWidget->addTab(m_techBrowser, tr("Tech Browser"));

    m_floorplanView = new GUI::FloorplanView(this);
    m_mainTabWidget->addTab(m_floorplanView, tr("Floorplan"));

    // create console
    m_console = new GUI::MMConsole(this);

    connect(m_console, &GUI::MMConsole::executeCommand, this, &MainWindow::onConsoleCommand);

    m_splitter = new QSplitter(Qt::Vertical, this);
    m_splitter->addWidget(m_mainTabWidget);
    m_splitter->addWidget(m_console);
    m_splitter->setStretchFactor(0,4);
    m_splitter->setStretchFactor(1,1);

    auto vLayout = new QVBoxLayout;
    vLayout->addWidget(m_splitter);

    QWidget *container = new QWidget();
    container->setLayout(vLayout);

    setCentralWidget(container);

    // Temporarily load the LEF and LIB files from the test
    // directory. This will only work is lunapnr is started
    // from the top level dir.
//#define nangate
#ifdef nangate

    std::ifstream libertyfile("test/files/nangate/ocl_functional.lib");
    if (!libertyfile.good())
    {
        doLog(LOG_WARN,"liberty file 'test/files/nangate/ocl_functional.lib' not found\n");
    }
    else
    {
        ChipDB::Liberty::Reader::load(&m_design, libertyfile);
        m_cellBrowser->setCellLib(&m_design.m_cellLib); // populate!
    }

    std::ifstream leffile("test/files/nangate/ocl.lef");
    if (!leffile.good())
    {
        doLog(LOG_WARN,"LEF file './test/files/nangate/ocl.lef' not found\n");
    }
    else
    {
        ChipDB::LEF::Reader::load(&m_design, leffile);
        m_cellBrowser->setCellLib(&m_design.m_cellLib); // populate!
    }

#endif


#if 0
    std::ifstream libertyfile("test/files/iit_stdcells/lib/tsmc018/signalstorm/iit018_stdcells.lib");
    if (!libertyfile.good())
    {
        doLog(LOG_WARN,"liberty file './test/files/iit_stdcells/lib/tsmc018/signalstorm/iit018_stdcells.lib' not found\n");
    }
    else
    {
        ChipDB::Liberty::Reader::load(&m_db.design(), libertyfile);
        m_cellBrowser->setDatabase(&m_db); // populate!
    }

    std::ifstream leffile("test/files/iit_stdcells/lib/tsmc018/lib/iit018_stdcells.lef");
    if (!leffile.good())
    {
        doLog(LOG_WARN,"LEF file './test/files/iit_stdcells/lib/tsmc018/lib/iit018_stdcells.lef' not found\n");
    }
    else
    {
        ChipDB::LEF::Reader::load(&m_db.design(), leffile);
        m_cellBrowser->setDatabase(&m_db); // populate!
    }

    std::ifstream verilogfile("test/files/verilog/nerv_tsmc018.v");
    if (!verilogfile.good())
    {
        doLog(LOG_WARN,"Verilog file './test/files/verilog/nerv_tsmc018.v' not found\n");
    }
    else
    {
        ChipDB::Verilog::Reader::load(&m_db.design(), verilogfile);
        auto nervModule = m_db.moduleLib().lookup("nerv");
        if (nervModule != nullptr)
        {
            m_floorplanView->setFloorplan(&nervModule->m_netlist);
        }
        //m_cellBrowser->setCellLib(&m_design.m_cellLib); // populate!

        LunaCore::SimpleCellPlacer::place(&nervModule->m_netlist, 
            ChipDB::Rect64{{1000,1000}, {650000,650000}}, 10000);

        auto hpwl = LunaCore::HPWLCalculator::calc(&nervModule->m_netlist);
        doLog(LOG_INFO, "HPWL = %lld\n", hpwl);
    }

#endif

    m_techBrowser->setDatabase(&m_db);
    m_cellBrowser->setDatabase(&m_db);

#if 0
    for(auto layer : m_db.techLib().m_layers)
    {
        if (layer != nullptr)
        {
            GUI::LayerRenderInfo info(layer->m_name);                
            m_db.m_layerRenderInfoDB.setRenderInfo(layer->m_name, info);
        }
    }
        
    {
        std::ifstream ifile("layers.json", std::ios::in);
        if (ifile.is_open())
        {
            std::stringstream buffer;
            buffer << ifile.rdbuf();
            m_db.m_layerRenderInfoDB.readJson(buffer.str());
        }
        else
        {
            doLog(LOG_ERROR, "Cannot open layers.json!\n");
        }
    }

#endif

    m_db.floorplan().m_regions.addListener(this);

    m_floorplanView->setDatabase(&m_db);
    m_floorplanView->update();

    connect(&m_guiUpdateTimer, &QTimer::timeout, this, &MainWindow::onGUIUpdateTimer);
    m_guiUpdateTimer.start(1000);

    m_lua.reset(new GUI::LuaWrapper(m_console, m_db));
    
    m_lua->run("print(\"Running \" .. _VERSION)\n");
}

MainWindow::~MainWindow()
{
}

void MainWindow::notify(int32_t userID, ssize_t index, NotificationType t)
{
    switch(userID)
    {
    case FloorplanNotificationID:
        m_floorplanDirty = true;
        break;
    case TechlibNotificationID:
        m_techLibDirty = true;
        break;
    case CellLibNotificationID:
        m_cellLibDirty = true;
        break;
    }
}

void MainWindow::createMenus()
{
    QMenu *fileMenu = menuBar()->addMenu(tr("&File"));
    fileMenu->addAction(m_runScriptAct);
    fileMenu->addSeparator();
    fileMenu->addAction(m_quitAct);

    QMenu *designMenu = menuBar()->addMenu(tr("&Design"));
    designMenu->addAction(m_loadVerilog);

    QMenu *techMenu = menuBar()->addMenu(tr("&Technology"));
    techMenu->addAction(m_importLEF);
    techMenu->addAction(m_importLIB);
    techMenu->addAction(m_importLayers);
    techMenu->addAction(m_exportLayers);

    QMenu *helpMenu = menuBar()->addMenu(tr("&Help"));
    helpMenu->addAction(m_aboutAct);
}

void MainWindow::createActions()
{
    m_runScriptAct = new QAction(tr("Run script..."), this);
    connect(m_runScriptAct, &QAction::triggered, this, &MainWindow::onRunScript);

    m_quitAct = new QAction(tr("&Quit"), this);
    connect(m_quitAct, &QAction::triggered, this, &MainWindow::onQuit);

    m_aboutAct = new QAction(tr("&About"), this);
    connect(m_aboutAct, &QAction::triggered, this, &MainWindow::onAbout);

    m_loadVerilog = new QAction(tr("&Load Verilog"), this);
    connect(m_loadVerilog, &QAction::triggered, this, &MainWindow::onLoadVerilog);

    m_importLEF = new QAction(tr("Import LEF"), this);
    connect(m_importLEF, &QAction::triggered, this, &MainWindow::onImportLEF);
    
    m_importLIB = new QAction(tr("Import LIB"), this);
    connect(m_importLIB, &QAction::triggered, this, &MainWindow::onImportLIB);
    
    m_importLayers = new QAction(tr("Import Layers"), this);
    connect(m_importLayers, &QAction::triggered, this, &MainWindow::onImportLayers);

    m_exportLayers = new QAction(tr("Export layers"), this);
    connect(m_exportLayers, &QAction::triggered, this, &MainWindow::onExportLayers);
}

void MainWindow::onQuit()
{
    QApplication::quit();
}

void MainWindow::onAbout()
{
    QMessageBox::aboutQt(this, "Luna place and route version " __DATE__ " " __TIME__ );
}

void MainWindow::onImportLEF()
{
    QString directory("");

    // Fixme: remember the last directory
    QString fileName = QFileDialog::getOpenFileName(this, tr("Import LEF file"), directory,
            tr("LEF file (*.lef *.tlef)"));
    
    if (!fileName.isEmpty())
    {
        std::ifstream leffile(fileName.toStdString());
        if (!leffile.good())
        {
            doLog(LOG_ERROR,"LEF file '%s' cannot be opened for reading\n", fileName.toStdString().c_str());
            QMessageBox::critical(this, tr("Error"), tr("The LEF file could not be opened for reading"), QMessageBox::Close);
            return;
        }

        if (!ChipDB::LEF::Reader::load(&m_db.design(), leffile))
        {
            doLog(LOG_ERROR,"LEF file '%s' contains errors\n", fileName.toStdString().c_str());
            QMessageBox::critical(this, tr("Error"), tr("The LEF file contains errors"), QMessageBox::Close);
        }

        m_techBrowser->refreshDatabase();
        m_cellBrowser->refreshDatabase();
    }
}

void MainWindow::onImportLIB()
{
    // Fixme: remember the last directory
    QString directory("");

    // Fixme: remember the last directory
    QString fileName = QFileDialog::getOpenFileName(this, tr("Import Liberty timing file"), directory,
            tr("LIB file (*.lib)"));
    
    if (!fileName.isEmpty())
    {
        std::ifstream libfile(fileName.toStdString());
        if (!libfile.good())
        {
            doLog(LOG_ERROR,"LIB file '%s' cannot be opened for reading\n", fileName.toStdString().c_str());
            QMessageBox::critical(this, tr("Error"), tr("The Liberty file could not be opened for reading"), QMessageBox::Close);
            return;
        }

        if (!ChipDB::Liberty::Reader::load(&m_db.design(), libfile))
        {
            doLog(LOG_ERROR,"LIB file '%s' contains errors\n", fileName.toStdString().c_str());
            QMessageBox::critical(this, tr("Error"), tr("The LIB file contains errors"), QMessageBox::Close);
        }

        m_techBrowser->refreshDatabase();
        m_cellBrowser->refreshDatabase();
    }    
}

void MainWindow::onImportLayers()
{
    // Fixme: remember the last directory
    QString directory("");

    // Fixme: remember the last directory
    QString fileName = QFileDialog::getOpenFileName(this, tr("Import layer setup"), directory,
            tr("JSON file (*.json)"));
    
    if (!fileName.isEmpty())
    {
        std::ifstream ifile(fileName.toStdString(), std::ios::in);
        if (ifile.is_open())
        {
            std::stringstream buffer;
            buffer << ifile.rdbuf();
            if(!m_db.m_layerRenderInfoDB.readJson(buffer.str()))
            {
                QMessageBox::critical(this, tr("Error"), tr("The Layer setup file contains errors"), QMessageBox::Close);
                doLog(LOG_ERROR, "Cannot read/parse Layer setup file!\n");
            }
        }
        else
        {
            doLog(LOG_ERROR, "Cannot open Layer setup file!\n");
        }
    }

    m_techBrowser->refreshDatabase();
}

void MainWindow::onExportLayers()
{
    QString fileName = QFileDialog::getSaveFileName(this, tr("Save Layer setup File"),
                           "layers.json",
                           tr("Layer file (*.json)"));

    if (!fileName.isEmpty())
    {
        auto json = m_db.m_layerRenderInfoDB.writeJson();
        std::ofstream ofile(fileName.toStdString());
        if (!ofile.is_open())
        {
            QMessageBox::critical(this, tr("Error"), tr("Cannot save Layer setup file"), QMessageBox::Close);
            doLog(LOG_ERROR, "Cannot save Layer setup file!\n");
        }
        else
        {
            ofile << json << "\n";
            doLog(LOG_VERBOSE, "Layer setup file saved!\n");
        }
    }
}

void MainWindow::onLoadVerilog()
{
    // Fixme: remember the last directory
    QString directory("");

    QString fileName = QFileDialog::getOpenFileName(this, tr("load Verilog netlist"), directory,
            tr("Verilog file (*.v)"));
    
    if (!fileName.isEmpty())
    {
        std::ifstream verilogfile(fileName.toStdString(), std::ios::in);
        if (verilogfile.is_open())
        {
            if (!ChipDB::Verilog::Reader::load(&m_db.design(), verilogfile))
            {
                QMessageBox::critical(this, tr("Error"), tr("Could not parse Verilog file"), QMessageBox::Close);
                doLog(LOG_ERROR, "Cannot read/parse verilog file!\n");
            }
        }
        else
        {
            QMessageBox::critical(this, tr("Error"), tr("Could not open Verilog file"), QMessageBox::Close);
            doLog(LOG_ERROR, "Cannot open verilog file!\n");
        }
        m_designBrowser->refreshDatabase();
    }   
}

void MainWindow::onConsoleCommand(const QString &cmd)
{
    if (m_lua)
    {
        m_console->disablePrompt();
        m_lua->run(cmd.toUtf8().data());
        m_console->enablePrompt();
    }
    else
    {
        doLog(LOG_ERROR, "LUA not available!\n");
    }
}

void MainWindow::onRunScript()
{
    QString directory("");

    // Fixme: remember the last directory
    QString fileName = QFileDialog::getOpenFileName(this, tr("Run LUA Script"), directory,
            tr("LUA script (*.lua)"));

    if ((!fileName.isEmpty()) && m_lua)
    {
        std::ifstream luafile(fileName.toStdString());
        if (!luafile.good())
        {
            doLog(LOG_ERROR,"Script file '%s' cannot be opened for reading\n", fileName.toStdString().c_str());
            QMessageBox::critical(this, tr("Error"), tr("The LUA script file could not be opened for reading"), QMessageBox::Close);
            return;
        }

        std::stringstream ss;
        ss << luafile.rdbuf();
        m_console->disablePrompt();
        m_lua->run(ss.str());
        m_console->enablePrompt();
    }
}

void MainWindow::onGUIUpdateTimer()
{
    if (m_floorplanView->isVisible() && m_floorplanDirty)
    {
        m_floorplanView->update();
        m_floorplanDirty = false;
    }
    if (m_techBrowser->isVisible() && m_techLibDirty)
    {
        m_techBrowser->update();
        m_techLibDirty = false;
    }
    if (m_cellBrowser->isVisible() && m_cellLibDirty)
    {
        m_cellBrowser->update();
        m_cellLibDirty = false;
    }        
}
