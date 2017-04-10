
#ifndef PLOT_H
#define PLOT_H

#include <QColor>
#include <QList>
#include <QAction>
#include <qwt_plot.h>
#include <qwt_plot_grid.h>
#include <qwt_plot_shapeitem.h>

#include "zoomer.h"
#include "scalezoomer.h"
#include "plotsnapshotoverlay.h"

#include <qwt_plot_directpainter.h>

class Plot : public QwtPlot
{
    Q_OBJECT

public:
    Plot(QWidget* parent = 0);
    ~Plot();
    void setAxis(bool autoScaled, double yMin = 0, double yMax = 1);

    QList<QAction*> menuActions();

    static QColor makeColor(unsigned int channelIndex);

private:
    bool isAutoScaled;
    double yMin, yMax;
    Zoomer zoomer;
    ScaleZoomer sZoomer;
    QwtPlotGrid grid;
    PlotSnapshotOverlay* snapshotOverlay;

    QAction _showGridAction;
    QAction _showMinorGridAction;
    QAction _unzoomAction;
    QAction _darkBackgroundAction;

    QwtPlotDirectPainter *d_directPainter;

    void resetAxes();

public slots:
    void showGrid(bool show = true);
    void showMinorGrid(bool show = true);
    void unzoom();
    void darkBackground(bool enabled = true);

    void flashSnapshotOverlay();

private slots:
    void unzoomed();
};

#endif // PLOT_H
