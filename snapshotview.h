
#ifndef SNAPSHOTVIEW_H
#define SNAPSHOTVIEW_H

#include <QMainWindow>
#include <QInputDialog>
#include <QFileDialog>
#include <QVector>
#include <QPointF>
#include <QPen>
#include <QCloseEvent>
#include <qwt_plot_curve.h>
#include <qwt_plot_marker.h>
#include <qwt_curve_fitter.h>

#include "plot.h"
#include "snapshot.h"

namespace Ui {
class SnapshotView;
}

class SnapshotView : public QMainWindow
{
    Q_OBJECT

public:
    explicit SnapshotView(QWidget *parent, Snapshot* snapshot);
    ~SnapshotView();

signals:
    void closed();

private:
    Ui::SnapshotView *ui;
    QList<QwtPlotCurve*> curves;
    Snapshot* _snapshot;
    QInputDialog renameDialog;

    void closeEvent(QCloseEvent *event);

    QPointF linefunc(QPointF point1, QPointF point2);
    QVector<QPointF> AnglePoint;
    double getlineY(QPointF lineSlope,double xValue);
    QVector<QPointF> linepara;
    double getlineX(QPointF lineSlope,double yValue);

    QList<QwtPlotCurve*> p_proclotmax;
    QList<QwtPlotCurve*> p_proclotmin;
    unsigned numOfChannels ;
    unsigned numOfSamples ;

    QList<QwtPlotCurve*> AngleCruve;

    QList<QwtPlotMarker*> R_marker;
    QList<QwtPlotMarker*> K_marker;
    QList<QwtPlotMarker*> MA_marker;

    QList<QwtPlotCurve*> p_clotmax;
    QList<QwtPlotCurve*> p_clotmin;

    QVector<QVector<QPointF> >  clotMaxval;
    QVector<QVector<QPointF> >   clotMinval;
    QVector<QVector<QPointF> >   BaseClotDataMax;
    QVector<QVector<QPointF> >   BaseClotDataMin;
    QVector<QPointF>  RPoint;
    QVector<QPointF>  m_vecMagData;
    QVector<QPointF>  m_vecMagTegData;
    QVector<QPointF>  m_vecArcMagTegData;
    QwtPlotCurve magCurve;

    QwtSplineCurveFitter *curveFitter;

    QwtPlotCurve Teg_Time_Curve;
    QwtPlotCurve Arc_Teg_Time_Curve;

    QPointF RMAGValuePoint;
    QPointF KMAGValuePoint;
    QPointF MAMAGValuePoint;
    QPointF AngleMAGValuePoint;

    QwtPlotMarker RMAGvalueMarkers;
    QwtPlotMarker MAMAGvalueMarkers;
    QwtPlotMarker KMAGvalueMarkers;
    QwtPlotCurve AngleMAGCruve;

   // QList<QwtPlotCurve*> curves;

private slots:
    void showRenameDialog();
    void renameSnapshot(QString name);
    void save();
    void on_actionClotData_triggered(bool checked);
    void on_actionImportMagData_triggered();
    void on_actionMagClotCurve_triggered();
};

#endif // SNAPSHOTVIEW_H
