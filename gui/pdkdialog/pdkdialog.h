// SPDX-FileCopyrightText: 2021-2023 Niels Moseley <asicsforthemasses@gmail.com>
//
// SPDX-License-Identifier: GPL-3.0-only

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

    constexpr int selected() const noexcept
    {
        return m_tileList->selected();
    }

private slots:
    void onSelectionChanged(int id);

protected:
    int m_selected{-1};
    std::vector<PDKInfo>    &m_pdks;
    
    PDKTileList *m_tileList{nullptr};
    QScrollArea *m_tileListScroll{nullptr};

    QDialogButtonBox *m_buttonBox{nullptr};
    
};

};
