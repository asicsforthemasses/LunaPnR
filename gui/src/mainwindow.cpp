// SPDX-FileCopyrightText: 2021-2022 Niels Moseley <asicsforthemasses@gmail.com>
//
// SPDX-License-Identifier: GPL-3.0-only

#include <QEvent>
#include <QAction>
#include <QTimer>
#include <QTabWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QApplication>
#include <QMessageBox>
#include <QFileDialog>
#include <QFontDialog>
#include <QThreadPool>

#include <QJsonDocument>
#include <QFile>
#include <QHeaderView>

#include <sstream>
#include <fstream>

#include "mainwindow.h"
#include "common/subprocess.h"

#include "common/tasklist.h"

#include "configurationdialog.h"

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent)
{
    m_db = std::make_shared<GUI::Database>();

    m_floorplanDirty = true;
    m_techLibDirty   = true;
    m_cellLibDirty   = true;

    Logging::setLogLevel(Logging::LogType::VERBOSE);

    createActions();
    createMenus();

    auto hMainLayout = new QHBoxLayout();

    m_projectManager = new GUI::ProjectManager(*m_db.get());
    m_projectSplitter = new QSplitter(Qt::Horizontal, this);

    // create tabs
    m_mainTabWidget = new QTabWidget(this);

    m_designBrowser = new GUI::DesignBrowser(this);
    m_designBrowser->setDatabase(m_db);
    m_mainTabWidget->addTab(m_designBrowser, tr("Design Browser"));

    m_cellBrowser = new GUI::CellBrowser(this);
    m_cellBrowser->setDatabase(m_db);
    m_mainTabWidget->addTab(m_cellBrowser, tr("Cell Browser"));

    m_techBrowser = new GUI::TechBrowser(this);
    m_techBrowser->setDatabase(m_db);
    m_mainTabWidget->addTab(m_techBrowser, tr("Tech Browser"));

    m_floorplanView = new GUI::FloorplanView(this);
    m_mainTabWidget->addTab(m_floorplanView, tr("Floorplan"));

    // create console
    m_console = new GUI::MMConsole(this);

    connect(m_console, &GUI::MMConsole::executeCommand, this, &MainWindow::onConsoleCommand);

    m_consoleHandler = std::make_unique<ConsoleLogOutputHandler>(m_console);
    Logging::setOutputHandler(m_consoleHandler.get());

    m_consoleSplitter = new QSplitter(Qt::Vertical, this);
    m_consoleSplitter->addWidget(m_mainTabWidget);
    m_consoleSplitter->addWidget(m_console);
    m_consoleSplitter->setStretchFactor(0,4);
    m_consoleSplitter->setStretchFactor(1,1);

    m_projectSplitter->addWidget(m_projectManager);
    m_projectSplitter->addWidget(m_consoleSplitter);
    m_projectSplitter->setStretchFactor(0,1);
    m_projectSplitter->setStretchFactor(1,4);

    hMainLayout->addWidget(m_projectSplitter);
    //hMainLayout->addWidget(m_consoleSplitter);

    QWidget *container = new QWidget();
    container->setLayout(hMainLayout);

    setCentralWidget(container);

    m_floorplanView->setDatabase(m_db);
    //m_floorplanView->update();

    m_db->cellLib()->addListener(this);
    m_db->moduleLib()->addListener(this);
    m_db->floorplan()->addListener(this);

    connect(&m_guiUpdateTimer, &QTimer::timeout, this, &MainWindow::onGUIUpdateTimer);
    m_guiUpdateTimer.start(1000);

    loadSettings();

    m_python = std::make_unique<GUI::Python>(m_db.get(),
        m_console);
    
    m_python->init();

    // install python console redirection
    m_python->setConsoleRedirect(
        [this](const char *txt, ssize_t strLen)
        {
            if ((txt == nullptr)  || (strLen <= 0))
            {
                return;
            }            

            std::string_view svTxt(txt, strLen);

            m_console->print(svTxt);
        },
        [this](const char *txt, ssize_t strLen)
        {
            if ((txt == nullptr)  || (strLen <= 0))
            {
                return;
            }            

            m_console->print(txt);
        }        
    );

    m_python->executeScript(R"(import sys; print("Python version"); print (sys.version); )");
    m_python->executeScript(R"(from Luna import *; from LunaExtra import *;)");

    connect(m_projectManager, &GUI::ProjectManager::onAction, this, &MainWindow::onProjectManagerAction);

    m_taskList = std::make_unique<GUI::TaskList>(m_projectManager);
}

MainWindow::~MainWindow()
{
}

void MainWindow::notify(ChipDB::ObjectKey index, NotificationType t)
{
    m_floorplanDirty = true;
    m_techLibDirty = true;
    m_cellLibDirty = true;

#if 0    
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
#endif
}

void MainWindow::createMenus()
{
    QMenu *fileMenu = menuBar()->addMenu(tr("&File"));
    fileMenu->addAction(m_loadProject);
    fileMenu->addAction(m_saveProject);
    fileMenu->addAction(m_saveProjectAs);

    fileMenu->addSeparator();
    fileMenu->addAction(m_runScriptAct);
    fileMenu->addAction(m_clearAct);
    fileMenu->addAction(m_consoleFontAct);
    fileMenu->addSeparator();
    fileMenu->addAction(m_exportLayers);
    fileMenu->addSeparator();
    fileMenu->addAction(m_configAct);
    fileMenu->addSeparator();
    fileMenu->addAction(m_quitAct);

    QMenu *helpMenu = menuBar()->addMenu(tr("&Help"));
    helpMenu->addAction(m_aboutAct);
}

void MainWindow::createActions()
{
    m_runScriptAct = new QAction(tr("Run script..."), this);
    connect(m_runScriptAct, &QAction::triggered, this, &MainWindow::onRunScript);

    m_clearAct = new QAction(tr("Clear database"), this);
    connect(m_clearAct, &QAction::triggered, this, &MainWindow::onClearDatabase);

    m_consoleFontAct = new QAction(tr("Set console font"), this);
    connect(m_consoleFontAct, &QAction::triggered, this, &MainWindow::onConsoleFontDialog);

    m_quitAct = new QAction(tr("&Quit"), this);
    m_quitAct->setShortcut(QKeySequence::Quit);
    connect(m_quitAct, &QAction::triggered, this, &MainWindow::onQuit);

    m_aboutAct = new QAction(tr("&About"), this);    
    connect(m_aboutAct, &QAction::triggered, this, &MainWindow::onAbout);

    m_loadProject = new QAction(tr("&Open Project"), this);
    m_loadProject->setShortcut(QKeySequence::Open);
    connect(m_loadProject, &QAction::triggered, this, &MainWindow::onLoadProject);

    m_saveProject = new QAction(tr("&Save Project"), this);
    m_saveProject->setShortcut(QKeySequence::Save);
    connect(m_saveProject, &QAction::triggered, this, &MainWindow::onSaveProject);

    m_saveProjectAs = new QAction(tr("&Save Project As ..."), this);
    m_saveProjectAs->setShortcut(QKeySequence::SaveAs);
    connect(m_saveProjectAs, &QAction::triggered, this, &MainWindow::onSaveProjectAs);

    m_exportLayers = new QAction(tr("Export layers"), this);
    connect(m_exportLayers, &QAction::triggered, this, &MainWindow::onExportLayers);

    m_configAct = new QAction(tr("Luna Configuration"), this);
    connect(m_configAct, &QAction::triggered, this, &MainWindow::onLunaConfig);
}

void MainWindow::saveSettings()
{
    QSettings settings;

    Logging::doLog(Logging::LogType::VERBOSE, "Saving settings to %s\n", settings.fileName().toStdString().c_str());

    settings.setValue("application/size", size());

    auto consoleColours = m_console->getColours();
    settings.setValue("console/bkcolour", consoleColours.m_bkCol.name(QColor::HexRgb));
    settings.setValue("console/promptcolour", consoleColours.m_promptCol.name(QColor::HexRgb));
    settings.setValue("console/errorcolour", consoleColours.m_errorCol.name(QColor::HexRgb));

    settings.setValue("console/font", m_console->font().family());
    settings.setValue("console/fontsize", m_console->font().pointSize());

    settings.setValue("opensta_location", QString::fromStdString(m_db->m_projectSetup.m_openSTALocation));
}

void MainWindow::loadSettings()
{
    QSettings settings;

    Logging::doLog(Logging::LogType::VERBOSE, "Loading settings from %s\n", settings.fileName().toStdString().c_str());

    m_console->setColours(
        QColor(settings.value("console/bkcolour", "#1d1f21").toString()),
        QColor(settings.value("console/promptcolour", "#c5c8c6").toString()),
        QColor(settings.value("console/errorcolour", "#a54242").toString())
    );

    QFont font;
    font.setFamily(settings.value("console/font", "Consolas").toString());
    font.setPointSize(settings.value("console/fontsize", "11").toInt());
    m_console->setFont(font);

    auto openStaLocation = settings.value("opensta_location", "/usr/local/bin/sta").toString();
    m_db->m_projectSetup.m_openSTALocation = openStaLocation.toStdString();
}

void MainWindow::onQuit()
{
    saveSettings();
    Logging::setOutputHandler(nullptr);
    QApplication::quit();
}

void MainWindow::onAbout()
{
    QMessageBox::aboutQt(this, "Luna place and route version " __DATE__ " " __TIME__ );
}

void MainWindow::onLoadProject()
{
    QString directory("");
    
    QString fileName = QFileDialog::getOpenFileName(this, tr("Load project file"), directory,
            tr("Luna project file (*.lpr)"));
    
    if (!fileName.isEmpty())
    {
        std::ifstream pfile(fileName.toStdString());
        if (!pfile.good() || (!m_db->m_projectSetup.readFromJSON(pfile)))
        {
            Logging::doLog(Logging::LogType::ERROR,"Project file '%s' cannot be opened for reading\n", fileName.toStdString().c_str());
            QMessageBox::critical(this, tr("Error"), tr("The project file could not be opened for reading"), QMessageBox::Close);
            return;
        }

        m_projectFileName = fileName;
        m_projectManager->repopulate();

        if (m_taskList)
        {
            m_taskList->executeToTask(*m_db.get(), "ReadAllFiles");
        }
    }
}

void MainWindow::onSaveProject()
{
    if (m_projectFileName.isEmpty())
    {
        onSaveProjectAs();
    }
    else
    {
        std::ofstream pfile(m_projectFileName.toStdString());
        if (!pfile.good() || (!m_db->m_projectSetup.writeToJSON(pfile)))
        {
            Logging::doLog(Logging::LogType::ERROR,"Project file '%s' cannot be saved\n", m_projectFileName.toStdString().c_str());
            QMessageBox::critical(this, tr("Error"), tr("The project file could not be saved"), QMessageBox::Close);
            return;
        }
    }
}

void MainWindow::onSaveProjectAs()
{
    auto fileName = QFileDialog::getSaveFileName(this, tr("Save project file"),
        "luna.lpr",
        tr("Luna project file (*.lpr)"));

    if (!fileName.isEmpty())
    {
        std::ofstream pfile(fileName.toStdString());
        if (!pfile.good() || (!m_db->m_projectSetup.writeToJSON(pfile)))
        {
            Logging::doLog(Logging::LogType::ERROR,"Project file '%s' cannot be saved\n", m_projectFileName.toStdString().c_str());
            QMessageBox::critical(this, tr("Error"), tr("The project file could not be saved"), QMessageBox::Close);
            return;
        }        
        m_projectFileName = fileName;
    }
}

void MainWindow::onExportLayers()
{
    QString fileName = QFileDialog::getSaveFileName(this, tr("Save Layer setup file"),
                           "tech.layers",
                           tr("Layer file (*.layers)"));

    if (!fileName.isEmpty())
    {
        auto json = m_db->m_layerRenderInfoDB.writeJson();
        std::ofstream ofile(fileName.toStdString());
        if (!ofile.is_open())
        {
            QMessageBox::critical(this, tr("Error"), tr("Cannot save Layer setup file"), QMessageBox::Close);
            Logging::doLog(Logging::LogType::ERROR, "Cannot save Layer setup file!\n");
        }
        else
        {
            ofile << json << "\n";
            Logging::doLog(Logging::LogType::VERBOSE, "Layer setup file saved!\n");
        }
    }
}

void MainWindow::onConsoleCommand(const QString &cmd)
{
    if (m_python)
    {
        m_console->disablePrompt();
        m_python->executeScript(cmd.toUtf8().data());
        m_console->enablePrompt();
    }
    else
    {
        Logging::doLog(Logging::LogType::ERROR, "Python not available!\n");
    }
}

void MainWindow::onRunScript()
{
    QString directory("");

    // Fixme: remember the last directory
    QString fileName = QFileDialog::getOpenFileName(this, tr("Run Python Script"), directory,
            tr("Python script (*.py)"));

    if ((!fileName.isEmpty()) && m_python)
    {
        std::ifstream pyFile(fileName.toStdString());
        if (!pyFile.good())
        {
            Logging::doLog(Logging::LogType::ERROR,"Script file '%s' cannot be opened for reading\n", fileName.toStdString().c_str());
            QMessageBox::critical(this, tr("Error"), tr("The Python script file could not be opened for reading"), QMessageBox::Close);
            return;
        }

        std::stringstream ss;
        ss << pyFile.rdbuf();

        m_console->disablePrompt();
        std::stringstream message;
        message << "\nRunning script " << fileName.toStdString() << "\n";
        m_console->print(message);

        m_python->executeScript(ss.str());

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

void MainWindow::onClearDatabase()
{
    m_db->clear();
    m_console->print("Database cleared");
}

void MainWindow::onConsoleFontDialog()
{
    m_console->setFont(QFontDialog::getFont(0, m_console->font()));
}

void MainWindow::onProjectManagerAction(QString actionName)
{
    if (!m_db)
    {
        m_console->print("Error: no database");
    }

    if (actionName != "NONE")
    {
#if 0        
        auto taskCallback = [this, &actionName](GUI::TaskList::CallbackInfo info)
        {   
            auto taskPtr = m_taskList.at(info.m_taskIdx);
            m_console->mtPrint(Logging::fmt("Task %u:%s callback\n", info.m_taskIdx, taskPtr->name().c_str()));
            auto event = new GUI::ProjectManagerEvent(QString::fromStdString(taskPtr->name()));
            QApplication::postEvent(m_projectManager, event);
        };
#endif
        if (m_taskList)
        {
            m_taskList->executeToTask(*m_db.get(), actionName.toStdString());
        }
    }
}

void MainWindow::onLunaConfig()
{
    GUI::ConfigurationDialog dialog(*m_db.get());
    dialog.exec();
}

ConsoleLogOutputHandler::ConsoleLogOutputHandler(GUI::MMConsole *console) : m_console(console)
{
}

void ConsoleLogOutputHandler::print(Logging::LogType t, const std::string &txt)
{
    if (m_console != nullptr)
    {
        m_console->mtPrint(txt);
    }
    else
    {
        std::cout << txt;
    }
}

void ConsoleLogOutputHandler::print(Logging::LogType t, const std::string_view &txt)
{
    if (m_console != nullptr)
    {
        m_console->mtPrint(txt);
    }
    else
    {
        std::cout << txt;
    }
}
