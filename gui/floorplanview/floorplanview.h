#pragma once

#include <QWidget>
#include "lunacore.h"

namespace GUI
{

class FloorplanView : public QWidget
{
    Q_OBJECT

public:
    explicit FloorplanView(QWidget *parent = nullptr);
    virtual ~FloorplanView();

    QSize sizeHint() const;

    // For now we use a netlist that holds instances
    // with positions as the floorplan..
    void setFloorplan(const ChipDB::Netlist *nl);

protected:
    void mousePressEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void wheelEvent(QWheelEvent *event) override;

    void paintEvent(QPaintEvent *event) override;

    void drawInstance(QPainter &p, const ChipDB::InstanceBase *ins);

    QRectF m_viewPort;  ///< viewport in floorplan coordinates

    const ChipDB::Netlist *m_netlist;
    bool  m_dirty;
};

};  // namespace