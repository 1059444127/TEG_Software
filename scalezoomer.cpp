
#include <QRectF>

#include "scalezoomer.h"

ScaleZoomer::ScaleZoomer(QwtPlot* plot, QwtPlotZoomer* zoomer) :
    QObject(plot),
    bottomPicker(plot->axisWidget(QwtPlot::xBottom), plot->canvas()),
    leftPicker(plot->axisWidget(QwtPlot::yLeft), plot->canvas())
{
    _plot = plot;
    _zoomer = zoomer;
    connect(&bottomPicker, &ScalePicker::picked, this, &ScaleZoomer::bottomPicked);
    connect(&leftPicker, &ScalePicker::picked, this, &ScaleZoomer::leftPicked);
}

void ScaleZoomer::setPickerPen(QPen pen)
{
    bottomPicker.setPen(pen);
    leftPicker.setPen(pen);
}

void ScaleZoomer::bottomPicked(double firstPos, double lastPos)
{
    QRectF zRect;
    if (lastPos > firstPos)
    {
        zRect.setLeft(firstPos);
        zRect.setRight(lastPos);
    }
    else
    {
        zRect.setLeft(lastPos);
        zRect.setRight(firstPos);
    }

    zRect.setBottom(_plot->axisScaleDiv(QwtPlot::yLeft).lowerBound());
    zRect.setTop(_plot->axisScaleDiv(QwtPlot::yLeft).upperBound());
    _zoomer->zoom(zRect);
}

void ScaleZoomer::leftPicked(double firstPos, double lastPos)
{
    QRectF zRect;
    if (lastPos > firstPos)
    {
        zRect.setBottom(firstPos);
        zRect.setTop(lastPos);
    }
    else
    {
        zRect.setBottom(lastPos);
        zRect.setTop(firstPos);
    }

    zRect.setLeft(_plot->axisScaleDiv(QwtPlot::xBottom).lowerBound());
    zRect.setRight(_plot->axisScaleDiv(QwtPlot::xBottom).upperBound());
    _zoomer->zoom(zRect);
}
