
#ifndef DATAFORMATPANEL_H
#define DATAFORMATPANEL_H

#include <QWidget>
#include <QButtonGroup>
#include <QTimer>
#include <QSerialPort>
#include <QList>
#include <QtGlobal>

//#include "framebuffer.h"
#include "signaldata.h"

namespace Ui {
class DataFormatPanel;
}

class DataFormatPanel : public QWidget
{
    Q_OBJECT

public:
    explicit DataFormatPanel(QSerialPort* port,
                             QList<SignalData*>* channelBuffers,
                             QList<SignalData*>* baseElasticchannelBuffers,
                             QList<SignalData*>* maxclotingBuffers,
                             QList<SignalData*>* minclotingBuffers,
                             QWidget *parent = 0);
    ~DataFormatPanel();

    unsigned numOfChannels();
    unsigned samplesPerSecond();
    bool skipByteEnabled(void); // true for binary formats
    static int count[10];
    static int clotingCount[10];
    static int isStartDetectFlag[10];
    static int isStopDetectFlag[10];

    QPointF linefunc(QPointF point1, QPointF point2);

public slots:

    void requestSkipByte();
    void pause(bool);
    void enableDemo(bool);
    void startBaseTest(bool);
    void startDetect(bool);

signals:
    void numOfChannelsChanged(unsigned);
    void samplesPerSecondChanged(unsigned);
    void skipByteEnabledChanged(bool);
    void dataAdded();
    void setRvalue(unsigned, QPointF, QPointF);
    void setKvalue(unsigned, QPointF);
    void setMAvalue(unsigned, QPointF, double);
    void setAnglevalue(unsigned, QPointF, QPointF);
    void sendMaxMinValue(unsigned, double,double);
    void sendBaseMaxValue(unsigned, double,double,double);
    void sendBaseMinValue(unsigned, double,double,double);
    void sendElasticMaxMinValue(unsigned, double,double);

private:
    enum NumberFormat
    {
        NumberFormat_uint8,
        NumberFormat_uint16,
        NumberFormat_uint32,
        NumberFormat_int8,
        NumberFormat_int16,
        NumberFormat_int32,
        NumberFormat_float,
        NumberFormat_ASCII
    };

    Ui::DataFormatPanel *ui;
    QButtonGroup numberFormatButtons;

    QSerialPort* serialPort;
    QList<SignalData*>* _channelBuffers;
    QList<SignalData*>* _baseElasticchannelBuffers;
    QList<SignalData*>* _maxclotingBuffers;
    QList<SignalData*>* _minclotingBuffers;
    unsigned int _numOfChannels;
    NumberFormat numberFormat;
    unsigned int sampleSize; // number of bytes in the selected number format
    bool skipByteRequested;
    bool startBaseTested;
    bool paused;
    bool isStartDetect;


    const int SPS_UPDATE_TIMEOUT = 1;  // second
    unsigned _samplesPerSecond;
    unsigned int sampleCount;
    QTimer spsTimer;

    // demo
    QTimer demoTimer;
    int demoCount;

    QPointF preRvaluePoint;
    QPointF RvaluePoint;
    QPointF KvaluePoint;
    QPointF MAvaluePoint;
    QPointF AnglevaluePoint;

    QPointF lineParaPoint;

    void selectNumberFormat(NumberFormat numberFormatId);

    // points to the readSampleAs function for currently selected number format
    double (DataFormatPanel::*readSample)();

    // note that serialPort should already have enough bytes present
    template<typename T> double readSampleAs();

    // `data` contains i th channels data
    void addChannelData(unsigned int channel, double* data, unsigned size);

    double getlineY(QPointF lineSlope, double xValue);

private slots:
    void onDataReady();      // used with binary number formats
    void onDataReadyASCII(); // used with ASCII number format
    void onNumberFormatButtonToggled(int numberFormatId, bool checked);
    void onNumOfChannelsSP(int value);
    void spsTimerTimeout();
    void demoTimerTimeout();
};

#endif // DATAFORMATPANEL_H
