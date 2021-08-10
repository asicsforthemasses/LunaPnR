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

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    virtual ~MainWindow();

public slots:
    void onQuit();
    void onAbout();
    
protected:
    QMenuBar    *m_menuBar;
    QSplitter   *m_splitter;
    QTabWidget  *m_mainTabWidget;
    
    GUI::MMConsole      *m_console;
    GUI::CellBrowser    *m_cellBrowser;
    GUI::TechBrowser    *m_techBrowser;
    GUI::FloorplanView  *m_floorplanView;

    ChipDB::Design      m_design;   ///< current design
};
