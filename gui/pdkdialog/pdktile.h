#pragma once
#include <QFrame>
#include <QLabel>
#include <QLineEdit>
#include <QMouseEvent>
#include "common/pdkinfo.h"

namespace GUI
{

class PDKTile : public QFrame
{
    Q_OBJECT
public:
    PDKTile(const PDKInfo &info, QWidget *parent = nullptr);

    constexpr void setID(int id) noexcept
    {
        m_id = id;
    }

    void mousePressEvent(QMouseEvent *e) override;
    void setSelected(bool selected = true);

signals:
    void clicked(int id);

protected:
    int    m_id{-1};
    int    m_defaultLineWidth{1};
    QLabel *m_title{nullptr};
    QLabel *m_date{nullptr};
    QLabel *m_version{nullptr};
};

};
