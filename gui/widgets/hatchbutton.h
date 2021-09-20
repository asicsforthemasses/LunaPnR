#pragma once

#include <QPushButton>
#include <QPixmap>

namespace GUI
{

class SelectHatchButton : public QPushButton
{
    Q_OBJECT
public:
    SelectHatchButton(QWidget *parent = nullptr);

    void setHatch(const QPixmap &hatch);
    QPixmap getHatch() const;

public slots:
    void changeHatch();

signals:
    void onHatchChanged();

protected:
    void paintEvent(QPaintEvent *event);

private:
    QPixmap m_pixmap;
};

};