
#include <QAction>
#include <QTimer>
#include <QTabWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QApplication>

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
