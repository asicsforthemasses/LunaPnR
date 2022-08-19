#pragma once

#include <QEvent>
#include <QWidget>
#include <QBoxLayout>
#include <vector>
#include "../common/projectsetup.h"
#include "../widgets/flatactiontile.h"
#include "filesetupmanager.h"

namespace GUI
{

class ProjectManager : public QWidget
{
    Q_OBJECT
public:
    ProjectManager(ProjectSetup *projectSetup, QWidget *parent = nullptr);

    void repopulate();

signals:
    void onAction(QString actionName);

protected:
    bool event(QEvent * event) override;
    void create();    
    
    QVBoxLayout *m_managerLayout = nullptr;
    ProjectSetup *m_projectSetup = nullptr;
    
    GUI::FileSetupManager *m_fileSetupManager = nullptr;
    std::vector<FlatActionTile*> m_actionTiles;
};

}