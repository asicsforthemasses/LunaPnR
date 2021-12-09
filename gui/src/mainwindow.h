#pragma once

#include <vector>
#include <string>

#include <QObject>
#include <QMainWindow>
#include <QMenu>
#include <QMenuBar>
#include <QTableWidget>
#include <QSplitter>
#include <QTimer>

#include "../console/mmconsole.h"
#include "../techbrowser/techbrowser.h"
#include "../cellbrowser/cellbrowser.h"
#include "../designbrowser/designbrowser.h"
#include "../floorplanview/floorplanview.h"

#include "lunacore.h"
#include "luawrapper.h"
#include "../common/database.h"


class MainWindow : public QMainWindow, public ChipDB::INamedStorageListener
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    virtual ~MainWindow();

    static const int32_t FloorplanNotificationID = 1;
    static const int32_t TechlibNotificationID = 2;
    static const int32_t CellLibNotificationID = 3;

    // called by database updates
    void notify(int32_t userID, ssize_t index = -1, NotificationType t = NotificationType::UNSPECIFIED) override;

public slots:
    void onQuit();
    void onAbout();
    void onLoadVerilog();
    void onImportLEF();
    void onImportLIB();
    void onImportLayers();
    void onExportLayers();
    void onRunScript();
    void onConsoleCommand(const QString &cmd);
    void onGUIUpdateTimer();
    
protected:
    void createMenus();
    void createActions();

    QAction *m_quitAct;    
    QAction *m_aboutAct;
    QAction *m_loadVerilog;
    QAction *m_importLEF;
    QAction *m_importLIB;
    QAction *m_importLayers;
    QAction *m_exportLayers;
    QAction *m_runScriptAct;

    //QMenuBar    *m_menuBar;
    QSplitter   *m_splitter;
    QTabWidget  *m_mainTabWidget;
    
    GUI::MMConsole      *m_console;
    GUI::CellBrowser    *m_cellBrowser;
    GUI::TechBrowser    *m_techBrowser;
    GUI::DesignBrowser  *m_designBrowser;
    GUI::FloorplanView  *m_floorplanView;

    GUI::Database m_db;

    bool m_floorplanDirty;
    bool m_techLibDirty;
    bool m_cellLibDirty;
    
    QTimer m_guiUpdateTimer;

    std::unique_ptr<GUI::LuaWrapper> m_lua;
};
