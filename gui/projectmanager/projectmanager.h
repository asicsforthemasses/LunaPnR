#pragma once
#include <QWidget>
#include <QBoxLayout>
#include "../common/projectsetup.h"

namespace GUI
{

class ProjectManager : public QWidget
{
    Q_OBJECT
public:
    ProjectManager(ProjectSetup *projectSetup, QWidget *parent = nullptr);

protected:
    void create();

    QVBoxLayout *m_managerLayout = nullptr;
    ProjectSetup *m_projectSetup = nullptr;
};

}