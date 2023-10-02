// SPDX-FileCopyrightText: 2021-2023 Niels Moseley <asicsforthemasses@gmail.com>
//
// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <vector>
#include <string>
#include <filesystem>

#include <QObject>
#include <QMainWindow>
#include <QMenu>
#include <QMenuBar>
#include <QTableWidget>
#include <QSplitter>
#include <QTimer>
#include <QSettings>

#include "../console/mmconsole.h"
#include "../techbrowser/techbrowser.h"
#include "../cellbrowser/cellbrowser.h"
#include "../designbrowser/designbrowser.h"
#include "../floorplanview/floorplanview.h"
#include "../projectmanager/projectmanager.h"
#include "../pdkinstaller/pdkinstaller.h"
#include "../pdkdialog/pdkdialog.h"

#include "lunacore.h"
#include "../common/database.h"
#include "../common/projectsetup.h"
#include "../common/tasklist.h"
#include "../common/pdkinfo.h"

#include "../python/pyluna_extra.h"

class ConsoleLogOutputHandler : public Logging::LogOutputHandler
{
public:
    ConsoleLogOutputHandler(GUI::MMConsole *console);

    void print(Logging::LogType t, const std::string &txt);
    void print(Logging::LogType t, const std::string_view &txt);

protected:
    GUI::MMConsole *m_console = nullptr;
};

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
    void notify(ChipDB::ObjectKey index = -1, NotificationType t = NotificationType::UNSPECIFIED) override;

public slots:
    void onQuit();
    void onAbout();
    void onAboutQt();

    void onLoadProject();
    void onSaveProject();
    void onSaveProjectAs();

    void onLunaConfig();

    void onInstallPDK();
    void onSelectPDK();

    void onExportLayers();
    void onRunScript();
    void onClearDatabase();
    void onConsoleCommand(const QString &cmd);
    void onConsoleFontDialog();
    void onGUIUpdateTimer();

    void onProjectManagerAction(QString action);

#if 0
    void onPlace();
    void onWriteDEF();
    void onWriteGDS2();
#endif

protected:
    void createMenus();
    void createActions();
    void saveSettings();
    void loadSettings();
    void scanPDKs();

    QAction *m_clearAct;
    QAction *m_quitAct;    
    QAction *m_aboutAct;
    QAction *m_aboutQtAct;

    QAction *m_configAct;

    QAction *m_loadProject;
    QAction *m_saveProject;
    QAction *m_saveProjectAs;

    QAction *m_installPDK;
    QAction *m_selectPDK;

    QAction *m_exportLayers;
    QAction *m_runScriptAct;
    QAction *m_consoleFontAct;

    QSplitter   *m_projectSplitter;
    QSplitter   *m_consoleSplitter;
    QTabWidget  *m_mainTabWidget;
    
    GUI::MMConsole      *m_console;
    GUI::CellBrowser    *m_cellBrowser;
    GUI::TechBrowser    *m_techBrowser;
    GUI::DesignBrowser  *m_designBrowser;
    GUI::FloorplanView  *m_floorplanView;
    GUI::ProjectManager *m_projectManager;
    
    std::unique_ptr<GUI::TaskList> m_taskList;
    
    std::filesystem::path       m_PDKRoot;
    std::vector<GUI::PDKInfo>   m_pdks;
    std::string                 m_selectedPDKName;

    QString m_projectFileName;
    std::shared_ptr<GUI::Database> m_db;

    bool m_floorplanDirty;
    bool m_techLibDirty;
    bool m_cellLibDirty;

    QTimer m_guiUpdateTimer;

    std::unique_ptr<ConsoleLogOutputHandler> m_consoleHandler;
    std::unique_ptr<GUI::Python> m_python;
};
