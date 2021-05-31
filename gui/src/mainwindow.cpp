
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
