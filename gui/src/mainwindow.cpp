
#include <QAction>
#include <QTimer>
#include <QTabWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QApplication>
#include <QMessageBox>

#include <fstream>

#include "mainwindow.h"

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent)
{
    setLogLevel(LOG_VERBOSE);

    m_menuBar = new QMenuBar();
    setMenuBar(m_menuBar);

    QMenu *fileMenu = new QMenu(tr("&File"));
    m_menuBar->addMenu(fileMenu);
    
    QAction *exitAction = new QAction(tr("&Quit"), this);
    exitAction->setShortcuts(QKeySequence::Quit);
    connect(exitAction, &QAction::triggered, this, &MainWindow::onQuit);
    fileMenu->addAction(exitAction);

    QMenu *helpMenu = new QMenu(tr("&Help"));
    m_menuBar->addMenu(helpMenu);

    QAction *aboutAction = new QAction(tr("&About"), this);
    connect(aboutAction, &QAction::triggered, this, &MainWindow::onAbout);
    helpMenu->addAction(aboutAction);

    // create tabs
    m_mainTabWidget = new QTabWidget(this);

    m_cellBrowser = new GUI::CellBrowser(this);
    m_cellBrowser->setCellLib(&m_design.m_cellLib);
    m_mainTabWidget->addTab(m_cellBrowser, "Cell Browser");

    m_techBrowser = new GUI::TechBrowser(this);
    m_techBrowser->setTechLib(&m_design.m_techLib);
    m_mainTabWidget->addTab(m_techBrowser, "Tech Browser");

    m_floorplanView = new GUI::FloorplanView(this);
    m_mainTabWidget->addTab(m_floorplanView, "Floorplan");

    // create console
    m_console = new GUI::MMConsole(this);

    m_splitter = new QSplitter(Qt::Vertical, this);
    m_splitter->addWidget(m_mainTabWidget);
    m_splitter->addWidget(m_console);
    m_splitter->setStretchFactor(0,3);
    m_splitter->setStretchFactor(1,2);

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

#else

    std::ifstream libertyfile("test/files/iit_stdcells/lib/tsmc018/signalstorm/iit018_stdcells.lib");
    if (!libertyfile.good())
    {
        doLog(LOG_WARN,"liberty file './test/files/iit_stdcells/lib/tsmc018/signalstorm/iit018_stdcells.lib' not found\n");
    }
    else
    {
        ChipDB::Liberty::Reader::load(&m_design, libertyfile);
        m_cellBrowser->setCellLib(&m_design.m_cellLib); // populate!
    }

    std::ifstream leffile("test/files/iit_stdcells/lib/tsmc018/lib/iit018_stdcells.lef");
    if (!leffile.good())
    {
        doLog(LOG_WARN,"LEF file './test/files/iit_stdcells/lib/tsmc018/lib/iit018_stdcells.lef' not found\n");
    }
    else
    {
        ChipDB::LEF::Reader::load(&m_design, leffile);
        m_cellBrowser->setCellLib(&m_design.m_cellLib); // populate!
    }

    std::ifstream verilogfile("test/files/verilog/nerv_tsmc018.v");
    if (!verilogfile.good())
    {
        doLog(LOG_WARN,"Verilog file './test/files/verilog/nerv_tsmc018.v' not found\n");
    }
    else
    {
        ChipDB::Verilog::Reader::load(&m_design, verilogfile);
        auto nervModule = m_design.m_moduleLib.lookup("nerv");
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

    for(auto layer : m_design.m_techLib.m_layers)
    {
        if (layer != nullptr)
        {
            GUI::LayerRenderInfo info(layer->m_name, layer->m_id);
            m_layerRenderInfoDB.addLayerInfo(info);
        }
    }

    m_techBrowser->setTechLib(&m_design.m_techLib);
    m_techBrowser->setLayerRenderInfo(&m_layerRenderInfoDB);
    m_cellBrowser->setLayerRenderInfoDB(&m_layerRenderInfoDB);

    m_floorplanView->update();

}

MainWindow::~MainWindow()
{
}


void MainWindow::onQuit()
{
    QApplication::quit();
}

void MainWindow::onAbout()
{
    QMessageBox::aboutQt(this, "Luna place and route version " __DATE__ " " __TIME__ );
}
