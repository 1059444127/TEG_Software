
#include <QRectF>
#include <QKeySequence>
#include <QColor>

#include "plot.h"
#include "utils.h"
#include <qwt_plot_layout.h>
#include <qwt_plot_canvas.h>
#include <qwt_plot_marker.h>
#include <qwt_plot_curve.h>
#include <qwt_plot_directpainter.h>
#include <qwt_curve_fitter.h>
#include <qwt_painter.h>
#include <qevent.h>



Plot::Plot(QWidget* parent) :
    QwtPlot(parent),
    zoomer(this->canvas(), false),
    sZoomer(this, &zoomer),
    _showGridAction(tr("网格"), this),
    _showMinorGridAction(tr("小网格"), this),
    _unzoomAction(tr("无缩放"), this),
    _darkBackgroundAction(tr("背景"), this)
{
    isAutoScaled = true;

    QObject::connect(&zoomer, &Zoomer::unzoomed, this, &Plot::unzoomed);

    zoomer.setZoomBase();
    grid.attach(this);

    showGrid(false);
    darkBackground(false);

  //  d_directPainter = new QwtPlotDirectPainter();   //采用逐渐显示的方式进行曲线的显示

 //   setAutoReplot( false );             //关闭自动重绘功能
 //   setCanvas( new Canvas() );          //设置绘画控件的画布

   // plotLayout()->setAlignCanvasToScales( true );



     setAxisTitle( QwtPlot::xBottom, "Time [s]" );
    // enableAxis(QwtPlot::xBottom);

    _showGridAction.setToolTip("Show Grid");
    _showMinorGridAction.setToolTip("Show Minor Grid");
    _unzoomAction.setToolTip("Unzoom the Plot");
    _darkBackgroundAction.setToolTip("Enable Dark Plot Background");

    _showGridAction.setShortcut(QKeySequence("G"));
    _showMinorGridAction.setShortcut(QKeySequence("M"));

    _showGridAction.setCheckable(true);
    _showMinorGridAction.setCheckable(true);
    _darkBackgroundAction.setCheckable(true);

    _showGridAction.setChecked(false);
    _showMinorGridAction.setChecked(false);
    _darkBackgroundAction.setChecked(false);

    _showMinorGridAction.setEnabled(false);

    connect(&_showGridAction, SELECT<bool>::OVERLOAD_OF(&QAction::triggered),
            this, &Plot::showGrid);
    connect(&_showGridAction, SELECT<bool>::OVERLOAD_OF(&QAction::triggered),
            &_showMinorGridAction, &QAction::setEnabled);
    connect(&_showMinorGridAction, SELECT<bool>::OVERLOAD_OF(&QAction::triggered),
            this, &Plot::showMinorGrid);
    connect(&_unzoomAction, &QAction::triggered, this, &Plot::unzoom);
    connect(&_darkBackgroundAction, SELECT<bool>::OVERLOAD_OF(&QAction::triggered),
            this, &Plot::darkBackground);

    snapshotOverlay = NULL;
}

Plot::~Plot()
{
    if (snapshotOverlay != NULL) delete snapshotOverlay;
}

void Plot::setAxis(bool autoScaled, double yAxisMin, double yAxisMax)
{
    this->isAutoScaled = autoScaled;

    if (!autoScaled)
    {
        yMin = yAxisMin;
        yMax = yAxisMax;
    }

    zoomer.zoom(0);
    resetAxes();
}

QList<QAction*> Plot::menuActions()
{
    QList<QAction*> actions;
    actions << &_showGridAction;
    actions << &_showMinorGridAction;
    actions << &_unzoomAction;
    actions << &_darkBackgroundAction;
    return actions;
}

void Plot::resetAxes()
{
    if (isAutoScaled)
    {
        setAxisAutoScale(QwtPlot::yLeft);
    }
    else
    {
        setAxisScale(QwtPlot::yLeft, yMin, yMax);
    }

    replot();
}

void Plot::unzoomed()
{
    setAxisAutoScale(QwtPlot::xBottom);
    //setAxis(QwtPlot::xBottom,0,7200);
    resetAxes();
}

void Plot::showGrid(bool show)
{
    grid.enableX(show);
    grid.enableY(show);
    replot();
}

void Plot::showMinorGrid(bool show)
{
    grid.enableXMin(show);
    grid.enableYMin(show);
    replot();
}

void Plot::unzoom()
{
    zoomer.zoom(0);
}

void Plot::darkBackground(bool enabled)
{
    QColor gridColor;
    if (enabled)
    {
        setCanvasBackground(QBrush(Qt::black));
        gridColor.setHsvF(0, 0, 0.25);
        grid.setPen(gridColor);
        zoomer.setRubberBandPen(QPen(Qt::white));
        zoomer.setTrackerPen(QPen(Qt::white));
        sZoomer.setPickerPen(QPen(Qt::white));
    }
    else
    {
        setCanvasBackground(QBrush(Qt::white));
        gridColor.setHsvF(0, 0, 0.80);
        grid.setPen(gridColor);
        zoomer.setRubberBandPen(QPen(Qt::black));
        zoomer.setTrackerPen(QPen(Qt::black));
        sZoomer.setPickerPen(QPen(Qt::black));
    }
    replot();
}

/*
  Below crude drawing demostrates how color selection occurs for
  given channel index

  0°                     <--Hue Value-->                           360°
  |* . o . + . o . * . o . + . o . * . o . + . o . * . o . + . o . |

  * -> 0-3
  + -> 4-7
  o -> 8-15
  . -> 16-31

 */
QColor Plot::makeColor(unsigned int channelIndex)
{
    auto i = channelIndex;

    if (i < 4)
    {
        return QColor::fromHsv(360*i/4, 255, 230);
    }
    else
    {
        double p = floor(log2(i));
        double n = pow(2, p);
        i = i - n;
        return QColor::fromHsv(360*i/n + 360/pow(2,p+1), 255, 230);
    }
}

void Plot::flashSnapshotOverlay()
{
    if (snapshotOverlay != NULL) delete snapshotOverlay;

    QColor color;
    if (_darkBackgroundAction.isChecked())
    {
        color = QColor(Qt::white);
    }
    else
    {
        color = QColor(Qt::black);
    }

    snapshotOverlay = new PlotSnapshotOverlay(this->canvas(), color);
    connect(snapshotOverlay, &PlotSnapshotOverlay::done,
            [this]()
            {
                delete snapshotOverlay;
                snapshotOverlay = NULL;
            });
}
