
#ifndef PORTCONTROL_H
#define PORTCONTROL_H

#include <QWidget>
#include <QButtonGroup>
#include <QSerialPort>
#include <QStringList>
#include <QToolBar>
#include <QAction>
#include <QComboBox>

#include "portlist.h"

namespace Ui {
class PortControl;
}

class PortControl : public QWidget
{
    Q_OBJECT

public:
    explicit PortControl(QSerialPort* port, QWidget* parent = 0);
    ~PortControl();

    QSerialPort* serialPort;

    QToolBar* toolBar();

private:
    Ui::PortControl *ui;

    QButtonGroup parityButtons;
    QButtonGroup dataBitsButtons;
    QButtonGroup stopBitsButtons;
    QButtonGroup flowControlButtons;

    QToolBar portToolBar;
    QAction openAction;
    QComboBox tbPortList;
    PortList portList;

public slots:
    void loadPortList();
    void loadBaudRateList();
    void togglePort();
    void selectPort(QString portName);
    void enableSkipByte(bool enabled = true);

    void selectBaudRate(QString baudRate);
    void selectParity(int parity); // parity must be one of QSerialPort::Parity
    void selectDataBits(int dataBits); // bits must be one of QSerialPort::DataBits
    void selectStopBits(int stopBits); // stopBits must be one of QSerialPort::StopBits
    void selectFlowControl(int flowControl); // flowControl must be one of QSerialPort::FlowControl

private slots:
    void openActionTriggered(bool checked);

    void onCbPortListActivated(int index);
    void onTbPortListActivated(int index);

signals:
    void skipByteRequested();
    void portToggled(bool open);
};

#endif // PORTCONTROL_H
