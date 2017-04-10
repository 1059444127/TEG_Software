
#include "portcontrol.h"
#include "ui_portcontrol.h"

#include <QSerialPortInfo>
#include <QKeySequence>
#include <QLabel>
#include <QtDebug>
#include "utils.h"

#define TBPORTLIST_MINWIDTH (200)

PortControl::PortControl(QSerialPort* port, QWidget* parent) :
    QWidget(parent),
    ui(new Ui::PortControl),
    portToolBar("Port Toolbar"),
    openAction(tr("打开"), this)
{
    ui->setupUi(this);

    serialPort = port;
    portToolBar.setSizePolicy(QSizePolicy::Fixed,QSizePolicy::Expanding);
    // 设置打开端口打开的工具栏按钮
    openAction.setCheckable(true);
    openAction.setShortcut(QKeySequence("F2"));
    openAction.setIcon(QPixmap(":/new/toolbar/toolbarImage/Open_Lock_508px_1183229_easyicon.net.png"));
    openAction.setToolTip("Open Port");
    QObject::connect(&openAction, &QAction::triggered,
                     this, &PortControl::openActionTriggered);

    portToolBar.addWidget(&tbPortList);
    portToolBar.addAction(&openAction);

    // 设定端口
    tbPortList.setMinimumWidth(TBPORTLIST_MINWIDTH);
    tbPortList.setModel(&portList);
 //   tbPortList.setSizePolicy(QSizePolicy::Fixed,QSizePolicy::Expanding);
    ui->cbPortList->setModel(&portList);
    QObject::connect(ui->cbPortList,
                     SELECT<int>::OVERLOAD_OF(&QComboBox::activated),
                     this, &PortControl::onCbPortListActivated);
    QObject::connect(&tbPortList,
                     SELECT<int>::OVERLOAD_OF(&QComboBox::activated),
                     this, &PortControl::onTbPortListActivated);
    QObject::connect(ui->cbPortList,
                     SELECT<const QString&>::OVERLOAD_OF(&QComboBox::activated),
                     this, &PortControl::selectPort);
    QObject::connect(&tbPortList,
                     SELECT<const QString&>::OVERLOAD_OF(&QComboBox::activated),
                     this, &PortControl::selectPort);

    // 更新端口
    QObject::connect(ui->pbReloadPorts, &QPushButton::clicked,
                     this, &PortControl::loadPortList);

    ui->pbOpenPort->setDefaultAction(&openAction);

    // 选择波特率
    QObject::connect(ui->cbBaudRate,
                     SELECT<const QString&>::OVERLOAD_OF(&QComboBox::activated),
                     this, &PortControl::selectBaudRate);

    // 选择校验位
    parityButtons.addButton(ui->rbNoParity, (int) QSerialPort::NoParity);
    parityButtons.addButton(ui->rbEvenParity, (int) QSerialPort::EvenParity);
    parityButtons.addButton(ui->rbOddParity, (int) QSerialPort::OddParity);

    QObject::connect(&parityButtons,
                     SELECT<int>::OVERLOAD_OF(&QButtonGroup::buttonClicked),
                     this, &PortControl::selectParity);

    // 选择数据位
    dataBitsButtons.addButton(ui->rb8Bits, (int) QSerialPort::Data8);
    dataBitsButtons.addButton(ui->rb7Bits, (int) QSerialPort::Data7);
    dataBitsButtons.addButton(ui->rb6Bits, (int) QSerialPort::Data6);
    dataBitsButtons.addButton(ui->rb5Bits, (int) QSerialPort::Data5);

    QObject::connect(&dataBitsButtons,
                     SELECT<int>::OVERLOAD_OF(&QButtonGroup::buttonClicked),
                     this, &PortControl::selectDataBits);

    // 选择停止位
    stopBitsButtons.addButton(ui->rb1StopBit, (int) QSerialPort::OneStop);
    stopBitsButtons.addButton(ui->rb2StopBit, (int) QSerialPort::TwoStop);

    QObject::connect(&stopBitsButtons,
                     SELECT<int>::OVERLOAD_OF(&QButtonGroup::buttonClicked),
                     this, &PortControl::selectStopBits);

    // 选择流控制
    flowControlButtons.addButton(ui->rbNoFlowControl,
                                 (int) QSerialPort::NoFlowControl);
    flowControlButtons.addButton(ui->rbHardwareControl,
                                 (int) QSerialPort::HardwareControl);
    flowControlButtons.addButton(ui->rbSoftwareControl,
                                 (int) QSerialPort::SoftwareControl);

    QObject::connect(&flowControlButtons,
                     SELECT<int>::OVERLOAD_OF(&QButtonGroup::buttonClicked),
                     this, &PortControl::selectFlowControl);

    // 跳过一个字节
    QObject::connect(ui->pbSkipByte, &QPushButton::clicked,
                     [this](){emit skipByteRequested();});

    loadPortList();
    loadBaudRateList();
    ui->cbBaudRate->setCurrentIndex(ui->cbBaudRate->findText("9600"));
}

PortControl::~PortControl()
{
    delete ui;
}

void PortControl::loadPortList()
{
    QString currentSelection = ui->cbPortList->currentData(PortNameRole).toString();
    portList.loadPortList();
    int index = portList.indexOf(currentSelection);
    if (index >= 0)
    {
        ui->cbPortList->setCurrentIndex(index);
        tbPortList.setCurrentIndex(index);
    }
}

void PortControl::loadBaudRateList()
{
    ui->cbBaudRate->clear();

    for (auto baudRate : QSerialPortInfo::standardBaudRates())
    {
        ui->cbBaudRate->addItem(QString::number(baudRate));
    }
}

void PortControl::selectBaudRate(QString baudRate)
{
    if (serialPort->isOpen())
    {
        if (!serialPort->setBaudRate(baudRate.toInt()))
        {
            qCritical() << "Can't set baud rate!";
        }
    }
}

void PortControl::selectParity(int parity)
{
    if (serialPort->isOpen())
    {
        if(!serialPort->setParity((QSerialPort::Parity) parity))
        {
            qCritical() << "Can't set parity option!";
        }
    }
}

void PortControl::selectDataBits(int dataBits)
{
    if (serialPort->isOpen())
    {
        if(!serialPort->setDataBits((QSerialPort::DataBits) dataBits))
        {
            qCritical() << "Can't set numer of data bits!";
        }
    }
}

void PortControl::selectStopBits(int stopBits)
{
    if (serialPort->isOpen())
    {
        if(!serialPort->setStopBits((QSerialPort::StopBits) stopBits))
        {
            qCritical() << "Can't set number of stop bits!";
        }
    }
}

void PortControl::selectFlowControl(int flowControl)
{
    if (serialPort->isOpen())
    {
        if(!serialPort->setFlowControl((QSerialPort::FlowControl) flowControl))
        {
            qCritical() << "Can't set flow control option!";
        }
    }
}

void PortControl::togglePort()
{
    if (serialPort->isOpen())
    {
        serialPort->close();
        qDebug() << "Closed port:" << serialPort->portName();
        emit portToggled(false);
    }
    else
    {
        //获取端口的名称
        QString portText = ui->cbPortList->currentText();
        QString portName;
        int portIndex = portList.indexOf(portText);
        if (portIndex < 0) // not in list, add to model and update the selections
        {
            portList.appendRow(new PortListItem(portText));
            ui->cbPortList->setCurrentIndex(portList.rowCount()-1);
            tbPortList.setCurrentIndex(portList.rowCount()-1);
            portName = portText;
        }
        else
        {
            // 从portlist获取端口名字
            portName = static_cast<PortListItem*>(portList.item(portIndex))->portName();
        }

        serialPort->setPortName(ui->cbPortList->currentData(PortNameRole).toString());

        // 打开端口
        if (serialPort->open(QIODevice::ReadWrite))
        {
            // 设置端口
            selectBaudRate(ui->cbBaudRate->currentText());
            selectParity((QSerialPort::Parity) parityButtons.checkedId());
            selectDataBits((QSerialPort::DataBits) dataBitsButtons.checkedId());
            selectStopBits((QSerialPort::StopBits) stopBitsButtons.checkedId());
            selectFlowControl((QSerialPort::FlowControl) flowControlButtons.checkedId());

            qDebug() << "Opened port:" << serialPort->portName();
            emit portToggled(true);
        }
    }
    openAction.setChecked(serialPort->isOpen());
}

void PortControl::selectPort(QString portName)
{
    portName = portName.split(" ")[0];

    if (portName != serialPort->portName())
    {
        if (serialPort->isOpen())
        {
            togglePort();
            //如果串口改变就打开另外一个串口
            togglePort();
        }
    }
}

void PortControl::enableSkipByte(bool enabled)
{
    ui->pbSkipByte->setDisabled(enabled);
}

QToolBar* PortControl::toolBar()
{
    return &portToolBar;
}

void PortControl::openActionTriggered(bool checked)
{
    togglePort();
}

void PortControl::onCbPortListActivated(int index)
{
    tbPortList.setCurrentIndex(index);
}

void PortControl::onTbPortListActivated(int index)
{
    ui->cbPortList->setCurrentIndex(index);
}
