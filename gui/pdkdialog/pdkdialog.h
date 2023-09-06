#pragma once
#include <vector>
#include "common/pdkinfo.h"
#include <QDialog>
#include <QScrollArea>

namespace GUI
{

class PDKDialog : public QDialog
{
    Q_OBJECT
public:
    PDKDialog(std::vector<PDKInfo> &pdks);

protected:
    std::vector<PDKInfo> &m_pdks;

    QScrollArea *m_tileList{nullptr};
};

};
