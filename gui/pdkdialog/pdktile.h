#pragma once
#include <QFrame>
#include <QLabel>
#include <QLineEdit>
#include "common/pdkinfo.h"

namespace GUI
{

class PDKTile : public QFrame
{
    Q_OBJECT
public:
    PDKTile(const PDKInfo &info, QWidget *parent = nullptr);

protected:
    QLineEdit *m_title{nullptr};
    QLineEdit *m_date{nullptr};
    QLineEdit *m_version{nullptr};
};

};
