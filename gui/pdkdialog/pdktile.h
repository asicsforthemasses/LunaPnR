#pragma once
#include <QFrame>
#include <QLabel>
#include <QLineEdit>
#include <QMouseEvent>
#include "common/pdkinfo.h"

namespace GUI
{

class PDKTile : public QWidget
{
    Q_OBJECT
public:
    PDKTile(const PDKInfo &info, QWidget *parent = nullptr);

    constexpr void setID(int id) noexcept
    {
        m_id = id;
    }

    void paintEvent(QPaintEvent *e) override;
    void mousePressEvent(QMouseEvent *e) override;
    void setSelected(bool selected = true);

signals:
    void clicked(int id);

protected:
    bool   m_selected{false};
    int    m_id{-1};
    QLabel *m_title{nullptr};
    QLabel *m_date{nullptr};
    QLabel *m_version{nullptr};
};

};
