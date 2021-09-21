#pragma once

#include <vector>
#include <string>

#include <QObject>
#include <QMainWindow>
#include <QMenu>
#include <QMenuBar>
#include <QTableWidget>
#include <QSplitter>

#include "../console/mmconsole.h"
#include "../techbrowser/techbrowser.h"
#include "../cellbrowser/cellbrowser.h"
#include "../floorplanview/floorplanview.h"

#include "lunacore.h"
#include "../common/database.h"

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    virtual ~MainWindow();

public slots:
    void onQuit();
    void onAbout();
    void onLoadVerilog();
    void onImportLEF();
    void onImportLIB();
    void onImportLayers();
    
protected:
    void createMenus();
    void createActions();

    QAction *m_quitAct;    
    QAction *m_aboutAct;
    QAction *m_loadVerilog;
    QAction *m_importLEF;
    QAction *m_importLIB;
    QAction *m_importLayers;

    //QMenuBar    *m_menuBar;
    QSplitter   *m_splitter;
    QTabWidget  *m_mainTabWidget;
    
    GUI::MMConsole      *m_console;
    GUI::CellBrowser    *m_cellBrowser;
    GUI::TechBrowser    *m_techBrowser;
    GUI::FloorplanView  *m_floorplanView;

    GUI::Database m_db;
};
