
#ifndef PLOTSNAPSHOTOVERLAY_H
#define PLOTSNAPSHOTOVERLAY_H

#include <QColor>
#include <QPainter>
#include <QTimer>
#include <qwt_widget_overlay.h>

// Draws a sort of flashing effect on plot widget when snapshot taken
class PlotSnapshotOverlay : public QwtWidgetOverlay
{
    Q_OBJECT

public:
    PlotSnapshotOverlay(QWidget* widget, QColor color);

protected:
    void drawOverlay(QPainter*) const;

signals:
    void done(); // indicates that animation completed

private:
    QColor _color;
    QTimer animTimer; // controls fading
    QTimer updateTimer; // need to force repaint the canvas
};

#endif // PLOTSNAPSHOTOVERLAY_H
