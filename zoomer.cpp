
#include "zoomer.h"
#include <qwt_plot.h>
#include <QtDebug>

Zoomer::Zoomer(QWidget* widget, bool doReplot) :
    QwtPlotZoomer(widget, doReplot)
{
    // do nothing
}

void Zoomer::zoom(int up)
{
    if (up == +1)
    {
        this->setZoomBase(this->plot());
    }

    QwtPlotZoomer::zoom(up);

    if(zoomRectIndex() == 0)
    {
        emit unzoomed();
    }
}
