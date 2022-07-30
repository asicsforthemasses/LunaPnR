#pragma once
#include <QWidget>
#include <QBoxLayout>
#include "../common/projectsetup.h"
#include "filesetupmanager.h"

namespace GUI
{

class ProjectManager : public QWidget
{
    Q_OBJECT
public:
    ProjectManager(ProjectSetup *projectSetup, QWidget *parent = nullptr);

    void repopulate();

protected slots:
    void onFloorplanSetup();
    void onCTSSetup();

signals:
    void onPlace();
    void onWriteDEF();
    void onWriteGDS2();

protected:
    void create();    

    QVBoxLayout *m_managerLayout = nullptr;
    ProjectSetup *m_projectSetup = nullptr;
    
    GUI::FileSetupManager *m_fileSetupManager = nullptr;
};

}