
#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QButtonGroup>
#include <QLabel>
#include <QString>
#include <QVector>
#include <QList>
#include <QSerialPort>
#include <QSignalMapper>
#include <QTimer>
#include <QColor>
#include <QtGlobal>
#include <QToolButton>
#include <qwt_plot_curve.h>
#include <qwt_plot_textlabel.h>
#include <qwt_plot_marker.h>

#include "portcontrol.h"
#include "commandpanel.h"
#include "dataformatpanel.h"
#include "ui_about_dialog.h"
//#include "framebuffer.h"
//#include "signaldata.h"
#include "snapshotmanager.h"
#include "maintenancedlg.h"
#include "scanbarcodedlg.h"
#include "resultshowdlg.h"


namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

    void messageHandler(QtMsgType type, const QMessageLogContext &context,
                        const QString &msg);

private:
    Ui::MainWindow *ui;

    QDialog aboutDialog;
    MaintenanceDlg maintenanceDlg;
    void setupAboutDialog();
    QSerialPort serialPort;
    PortControl portControl;
    QToolButton *pauseBtn;


    unsigned int numOfSamples;

    QList<QwtPlotCurve*> curves;
    QList<QwtPlotCurve*> baseAndElaticscurves;
    QList<QwtPlotCurve*> maxClotCurves;
    QList<QwtPlotCurve*> minClotCurves;
    QList<QwtPlotMarker*> RvalueMarkers;
    QList<QwtPlotMarker*> KvalueMarkers;
    QList<QwtPlotMarker*> MAvalueMarkers;

    QList<QwtPlotCurve*>  AngleCurves;
    // Note: FrameBuffer s are owned by their respective QwtPlotCurve s.
    QList<SignalData*> channelBuffers;
    QList<SignalData*> baseElasticBuffers;
    QList<SignalData*> maxclotingBuffers;
    QList<SignalData*> minclotingBuffers;
    QLabel spsLabel;

    CommandPanel commandPanel;
    DataFormatPanel dataFormatPanel;

    SnapshotManager snapshotMan;

    QwtPlotTextLabel demoIndicator;
    bool isDemoRunning();

    bool isComSetOpen;

    QVector<QPointF> preRValuePoint;
    QVector<QPointF> RValuePoint;
    QVector<QPointF> KValuePoint;
    QVector<QPointF> MAValuePoint;
    QVector<QPointF> AngleValuePoint;

    double getlineX(QPointF lineSlope, double yValue);

    static int startTimeArray[10];
    static int stopTimeArray[10];

    ScanBarCodeDlg barcodeDlg;
    ResultShowDlg resultshowDlg;


signals:


private Q_SLOTS:
    void onPortToggled(bool open);
    void onPortError(QSerialPort::SerialPortError error);

    void onNumOfSamplesChanged(int value);
    void onAutoScaleChecked(bool checked);
    void onYScaleChanged();
    void onRangeSelected();
    void onNumOfChannelsChanged(unsigned value);
    void onSetRvalue(unsigned channel, QPointF preRvaluePoint,QPointF RvaluePoint);
    void onSetKvalue(unsigned channel, QPointF KvaluePoint);
    void onSetMAvalue(unsigned channel, QPointF MAvaluePoint, double baselineValue);
    void onSetAnglevalue(unsigned channel, QPointF AnglevaluePoint, QPointF lineParaPoint);

    void clearPlot();

    void onSpsChanged(unsigned sps);

    void enableDemo(bool enabled);

    void onExportCsv();
    void on_actionComSet_triggered();
    void setBaseMaxValue(unsigned channel, double maxValue,double minValue,double startTimes);
    void setBaseMinValue(unsigned channel, double maxValue,double minValue,double stopTimes);
    void setElasticMaxMinValue(unsigned channel, double maxValue,double minValue);

#ifndef QT_NO_PRINTER
    void on_actionPrint_triggered();
#endif

    void on_actionReport_triggered();
    void on_action_Preset_triggered();
    void on_pbHideComFrm_clicked();

    void on_actionStop_triggered();
    void on_action_maintance_triggered();
    void on_actionBarCode_triggered();
    void on_actionPatient_triggered();
};

#endif // MAINWINDOW_H
