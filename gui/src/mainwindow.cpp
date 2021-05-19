
#include <QAction>
#include <QTimer>
#include <QTabWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QApplication>
#include <QMessageBox>

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

    QWidget *container = new QWidget();    
    setCentralWidget(container);
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
