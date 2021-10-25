#pragma once

#include <QWidget>
#include "common/guihelpers.h"
#include "common/database.h"

namespace GUI
{

struct FloorplanOverlayBase
{
    virtual void paint(QPainter &painter) = 0;
};

class FloorplanView : public QWidget
{
    Q_OBJECT

public:
    explicit FloorplanView(QWidget *parent = nullptr);
    virtual ~FloorplanView();

    QSize sizeHint() const override;

    /** set the database which contains the floorplan object */
    void setDatabase(Database *db);

    /** set an overlay object that will draw on to of the floorplan */
    void setOverlay(FloorplanOverlayBase *overlay = nullptr);

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

    Viewport m_viewPort;    

    enum class MouseState
    {
        None,
        Dragging
    } m_mouseState;

    ChipDB::Rect64 m_viewPortRef;
    QPoint m_mouseDownPos;

    FloorplanOverlayBase* m_overlay;

    Database *m_db;
    bool  m_dirty;
};

};  // namespace