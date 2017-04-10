
#ifndef ZOOMER_H
#define ZOOMER_H

#include <qwt_plot_zoomer.h>

class Zoomer : public QwtPlotZoomer
{
    Q_OBJECT

public:
    Zoomer(QWidget *, bool doReplot=true);
    void zoom(int up);

signals:
    void unzoomed();
};

#endif // ZOOMER_H
