#pragma once

#include <QWidget>
#include "common/database.h"

namespace GUI
{

class FloorplanView : public QWidget
{
    Q_OBJECT

public:
    explicit FloorplanView(QWidget *parent = nullptr);
    virtual ~FloorplanView();

    QSize sizeHint() const override;

    /** set the database which contains the floorplan object */
    void setDatabase(Database *db);

protected:
    void mousePressEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void wheelEvent(QWheelEvent *event) override;

    void paintEvent(QPaintEvent *event) override;

    void resizeEvent(QResizeEvent *event) override;

    void drawInstances(QPainter &p);
    void drawRegions(QPainter &p);
    void drawRows(QPainter &p, const ChipDB::Region *region);

    void drawCell(QPainter &p, const ChipDB::InstanceBase *ins);
    void drawPin(QPainter &p, const ChipDB::InstanceBase *ins);

    QRectF m_viewPort;  ///< viewport in floorplan coordinates

    enum class MouseState
    {
        None,
        Dragging
    } m_mouseState;

    QRectF m_viewPortRef;
    QPoint m_mouseDownPos;

    Database *m_db;
    bool  m_dirty;
};

};  // namespace