#pragma once

#include <QDialog>
#include "../common/hatchlibrary.h"
#include "../common/msvcfix.h"

namespace GUI
{

class HatchDialog : public QDialog
{
    Q_OBJECT

public:
    HatchDialog(HatchLibrary &hatchLibrary, QWidget *parent = nullptr, Qt::WindowFlags f = Qt::WindowFlags());

    ssize_t getHatchIndex() const noexcept
    {
        return m_hatchIndex;
    }

protected slots:    
    void onHatchClicked(int index);

protected:
    ssize_t m_hatchIndex;
};

};