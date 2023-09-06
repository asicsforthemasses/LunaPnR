#pragma once
#include <vector>
#include "common/pdkinfo.h"
#include "pdktile.h"
#include <QDialog>
#include <QDialogButtonBox>
#include <QScrollArea>

namespace GUI
{

class PDKDialog : public QDialog
{
    Q_OBJECT
public:
    PDKDialog(std::vector<PDKInfo> &pdks);

private slots:
    void onTileClicked(int id);

protected:
    int m_selected{-1};
    std::vector<PDKInfo>    &m_pdks;
    std::vector<PDKTile*>   m_pdkTiles;
    QDialogButtonBox        *m_buttonBox{nullptr};
    QScrollArea *m_tileList{nullptr};
};

};
