#pragma once
#include <QFrame>
#include <QBoxLayout>

namespace GUI
{

class BlockFrame : public QFrame
{
public:
    BlockFrame(QWidget *parent = nullptr);

    void addWidget(QWidget *widget, int stetch = 0, Qt::Alignment alignment = Qt::Alignment());

protected:
    QVBoxLayout *m_layout;
};

};
