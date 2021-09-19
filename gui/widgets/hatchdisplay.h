#pragma once

#include <QScrollArea>
#include "../common/hatchlibrary.h"

namespace GUI
{

class HatchDisplay : public QScrollArea
{
    Q_OBJECT

public:
    HatchDisplay(HatchLibrary &hatchLibrary, QWidget *parent = nullptr);

signals:
    void clicked(int hatchindex);

protected slots:
    void onHatchClick();

protected:
    HatchLibrary &m_hatchLibrary;
};

};