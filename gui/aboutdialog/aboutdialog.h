// SPDX-FileCopyrightText: 2021-2022 Niels Moseley <asicsforthemasses@gmail.com>
//
// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <QDialog>
#include <QTabWidget>
#include <QDialogButtonBox>

namespace GUI
{

class AboutDialog : public QDialog
{
    Q_OBJECT
public:
    AboutDialog(QWidget *parent = nullptr);

protected:
    QTabWidget *m_tabWidget;
    QDialogButtonBox *m_buttonBox;
};

class AboutGeneralTab : public QWidget
{
    Q_OBJECT

public:
    explicit AboutGeneralTab(QWidget *parent = nullptr);
};

class AboutQtTab : public QWidget
{
    Q_OBJECT

public:
    explicit AboutQtTab(QWidget *parent = nullptr);
};

};