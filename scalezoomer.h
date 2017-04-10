
#ifndef SCALEZOOMER_H
#define SCALEZOOMER_H

#include <QObject>
#include <QPen>
#include <qwt_plot.h>
#include <qwt_plot_zoomer.h>

#include "scalepicker.h"

class ScaleZoomer : public QObject
{
    Q_OBJECT

public:
    ScaleZoomer(QwtPlot*, QwtPlotZoomer*);
    void setPickerPen(QPen pen);

private:
    QwtPlot* _plot;
    QwtPlotZoomer* _zoomer;
    ScalePicker bottomPicker;
    ScalePicker leftPicker;

private slots:
    void bottomPicked(double firstPos, double lastPos);
    void leftPicked(double firstPos, double lastPos);
};

#endif /* SCALEZOOMER_H */
