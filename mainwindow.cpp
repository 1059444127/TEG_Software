
#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QByteArray>
#include <QApplication>
#include <QFileDialog>
#include <QFile>
#include <QTextStream>
#include <QMenu>
#include <QtDebug>
#include <limits.h>
#include <cmath>
#include <iostream>
#include <QtPrintSupport/QPrinter>
#include <QtPrintSupport/QPrintDialog>
#include <qwt_plot_renderer.h>


#include "plot.h"
#include "presetdlg.h"

#include "utils.h"


#if defined(Q_OS_WIN) && defined(QT_STATIC)
#include <QtPlugin>
Q_IMPORT_PLUGIN(QWindowsIntegrationPlugin)
#endif

struct Range
{
    double rmin;
    double rmax;
};

int MainWindow::startTimeArray[10]={0};
int MainWindow::stopTimeArray[10]={0};

Q_DECLARE_METATYPE(Range);

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    aboutDialog(this),
    maintenanceDlg(this),
    portControl(&serialPort),
    commandPanel(&serialPort),
    dataFormatPanel(&serialPort, &channelBuffers,&baseElasticBuffers, &maxclotingBuffers,&minclotingBuffers),
    snapshotMan(this, &channelBuffers),
    isComSetOpen(false)
{
    ui->setupUi(this);
    this->setWindowTitle(tr("重庆大学血栓弹力图仪"));
    ui->comsetFrame->hide();
    setToolButtonStyle( Qt::ToolButtonTextUnderIcon );

    /******************开始编写时间时间：2015年11月24日******************/


    /********************终止编写时间：2015年11月24日*********************/

    /********************将串口、数据、发送命令全部挂在TABWIDGET**********************************/
    ui->tabWidget->insertTab(0, &portControl, "端口");  //插入串口控制的挂件
    ui->tabWidget->insertTab(1, &dataFormatPanel, "数据");  //插入数据格式的挂件
    ui->tabWidget->insertTab(3, &commandPanel, "命令");         //串口发送挂件
    ui->tabWidget->setCurrentIndex(0);

    ui->baseAdjtableWidget->horizontalHeader()->resizeSection(0,50); //设置表头第一列的宽度为150
    ui->baseAdjtableWidget->horizontalHeader()->setStyleSheet("QHeaderView::section{background:skyblue;}"); //设置表头背景色

    ui->elastictableWidget->horizontalHeader()->resizeSection(0,50); //设置表头第一列的宽度为150
    ui->elastictableWidget->horizontalHeader()->setStyleSheet("QHeaderView::section{background:skyblue;}"); //设置表头背景色

    for(int i = 0 ; i < 8; i++){

        QString str = QString::number(i+1,'f',0);
        ui->baseAdjtableWidget->setItem(i,0,new QTableWidgetItem(str));
        ui->elastictableWidget->setItem(i,0,new QTableWidgetItem(str));
    }

    addToolBar(portControl.toolBar());                  //在工具栏中添加串口选择控件

    ui->twCruve->insertTab(2,&maintenanceDlg,"系统维护");


    ui->mainToolBar->insertAction(ui->actionCase,snapshotMan.takeSnapshotAction());
   // ui->mainToolBar->addAction(snapshotMan.takeSnapshotAction());  //在工具栏中添加截屏控件
    ui->menuBar->insertMenu(ui->menu_Help->menuAction(), snapshotMan.menu());  //在工具栏在添加截屏菜单栏

    setupAboutDialog();

    // 初始化曲线视图
    for (auto a : ui->plot->menuActions())  // 将Plot类中的相关动作添加进来
    {
        ui->menuView->addAction(a);
    }
    ui->menuView->addSeparator();
    for (auto a : ui->clotPlot->menuActions())  // 将Plot类中的相关动作添加进来
    {
        ui->menuView->addAction(a);
    }

    ui->menuView->addSeparator();

    QMenu* tbMenu = ui->menuView->addMenu("Toolbars");
    tbMenu->addAction(ui->mainToolBar->toggleViewAction());
    tbMenu->addAction(portControl.toolBar()->toggleViewAction());



    // 初始化界面的信号


    // 菜单信号

    QObject::connect(ui->actionHelpAbout, &QAction::triggered,  // 显示关于按钮
              &aboutDialog, &QWidget::show);

    QObject::connect(ui->actionExportCsv, &QAction::triggered,   //数据存储
                     this, &MainWindow::onExportCsv);

    ui->actionQuit->setShortcutContext(Qt::ApplicationShortcut);  //停止按钮


    QObject::connect(ui->actionQuit, &QAction::triggered,
                     this, &MainWindow::close);                     //关闭应用程序

    //端口控制信号
    QObject::connect(&portControl, &PortControl::portToggled,
                     this, &MainWindow::onPortToggled);             //串口打开的槽函数

    QObject::connect(&portControl, &PortControl::skipByteRequested,
                     &dataFormatPanel, &DataFormatPanel::requestSkipByte);

    QObject::connect(ui->spNumOfSamples, SELECT<int>::OVERLOAD_OF(&QSpinBox::valueChanged),
                     this, &MainWindow::onNumOfSamplesChanged);   //曲线显示的样本点的数据改变槽函数

    QObject::connect(ui->cbAutoScale, &QCheckBox::toggled,
                     this, &MainWindow::onAutoScaleChecked); //设置自动检测坐标轴的值

    QObject::connect(ui->spYmin, SIGNAL(valueChanged(double)),
                     this, SLOT(onYScaleChanged()));             //设置显示的最小值

    QObject::connect(ui->spYmax, SIGNAL(valueChanged(double)),
                     this, SLOT(onYScaleChanged()));                  //设置显示最大值



    QObject::connect(ui->actionClear, SIGNAL(triggered(bool)),
                     this, SLOT(clearPlot()));          //清屏

    QObject::connect(snapshotMan.takeSnapshotAction(), &QAction::triggered,
                     ui->plot, &Plot::flashSnapshotOverlay);
    //截屏
    QObject::connect(snapshotMan.takeSnapshotAction(), &QAction::triggered,
                     ui->clotPlot, &Plot::flashSnapshotOverlay);

    // 初始化端口信息
    QObject::connect(&(this->serialPort), SIGNAL(error(QSerialPort::SerialPortError)),
                     this, SLOT(onPortError(QSerialPort::SerialPortError)));   //串口错误信息

    // 设置Y轴
    ui->spYmin->setRange((-1) * std::numeric_limits<double>::max(),
                         std::numeric_limits<double>::max());

    ui->spYmax->setRange((-1) * std::numeric_limits<double>::max(),
                         std::numeric_limits<double>::max());

    // 初始化信息格式
    QObject::connect(&dataFormatPanel, &DataFormatPanel::dataAdded,
                     ui->plot, &QwtPlot::replot);    //当数据读取的时候在绘图QWT进行绘图

    QObject::connect(&dataFormatPanel, &DataFormatPanel::dataAdded,
                     ui->plot_2, &QwtPlot::replot);    //当数据读取的时候在绘图QWT进行绘图

    QObject::connect(&dataFormatPanel, &DataFormatPanel::dataAdded,
                     ui->plot_3, &QwtPlot::replot);    //当数据读取的时候在绘图QWT进行绘图

    QObject::connect(&dataFormatPanel, &DataFormatPanel::dataAdded,
                     ui->plot_4, &QwtPlot::replot);    //当数据读取的时候在绘图QWT进行绘图

    QObject::connect(&dataFormatPanel, &DataFormatPanel::dataAdded,
                     ui->BaseandElasticPlot, &QwtPlot::replot);    //当数据读取的时候在绘图QWT进行绘图

    QObject::connect(&dataFormatPanel, &DataFormatPanel::dataAdded,
                     ui->clotPlot, &QwtPlot::replot);    //当数据读取的时候在绘图QWT进行绘图

    QObject::connect(&dataFormatPanel,&DataFormatPanel::setRvalue,
                     this,&MainWindow::onSetRvalue);             //设置R值
    QObject::connect(&dataFormatPanel, &DataFormatPanel::setKvalue,
                     this,&MainWindow::onSetKvalue);                //设置K值
    QObject::connect(&dataFormatPanel,&DataFormatPanel::setMAvalue,
                     this,&MainWindow::onSetMAvalue);               //设置MA值
    QObject::connect(&dataFormatPanel,&DataFormatPanel::setAnglevalue,
                     this,&MainWindow::onSetAnglevalue);            //设置Angle值

    QObject::connect(ui->actionPause, &QAction::triggered,
                     &dataFormatPanel, &DataFormatPanel::pause);        //暂停按钮

    QObject::connect(ui->actionStart,&QAction::triggered,
                     &dataFormatPanel, &DataFormatPanel::startDetect);  //开始检测


    numOfSamples = ui->spNumOfSamples->value();                     //采取样本点的大小
    unsigned numOfChannels = dataFormatPanel.numOfChannels();           //采用的通道数

    QObject::connect(&dataFormatPanel,
                     &DataFormatPanel::numOfChannelsChanged,
                     this,
                     &MainWindow::onNumOfChannelsChanged);          //通道变化的槽函数

    // 初始化通道和曲线
    for (unsigned int i = 0; i < numOfChannels; i++)                //各个通道的绘图
    {

        //原始曲线的数据和曲线
        channelBuffers.append(new SignalData());      //各个通道的缓存        
        curves.append(new QwtPlotCurve());
        curves[i]->setSamples(channelBuffers[i]);
        curves[i]->setPen(Plot::makeColor(i),2.0,Qt::SolidLine);

        switch (i%4) {
        case 0:
            curves[i]->attach(ui->plot);
            break;
        case 1:
            curves[i]->attach(ui->plot_2);
            break;
        case 2:
            curves[i]->attach(ui->plot_3);
            break;
        case 3:
            curves[i]->attach(ui->plot_4);
            break;
        default:
            break;
        }


        baseElasticBuffers.append(new SignalData());
        baseAndElaticscurves.append(new QwtPlotCurve());
        baseAndElaticscurves[i]->setSamples(baseElasticBuffers[i]);
        baseAndElaticscurves[i]->setPen(Plot::makeColor(i),1.0,Qt::SolidLine);
        baseAndElaticscurves[i]->attach(ui->BaseandElasticPlot);



        //最大值得轮廓曲线和数据
        maxclotingBuffers.append(new SignalData());
        maxClotCurves.append(new QwtPlotCurve());
        maxClotCurves[i]->setSamples(maxclotingBuffers[i]);
        maxClotCurves[i]->setPen(Plot::makeColor(i),1.0,Qt::SolidLine);
        maxClotCurves[i]->attach(ui->clotPlot);

        //最小值的轮廓曲线和数据
        minclotingBuffers.append(new SignalData());       
        minClotCurves.append(new QwtPlotCurve()); 
        minClotCurves[i]->setSamples(minclotingBuffers[i]);
        minClotCurves[i]->setPen(Plot::makeColor(i),1.0,Qt::SolidLine);
        minClotCurves[i]->attach(ui->clotPlot);

        //R值标记

        RvalueMarkers.append(new QwtPlotMarker());
        RvalueMarkers[i]->setLineStyle(QwtPlotMarker::VLine);
        RvalueMarkers[i]->setLabelAlignment( Qt::AlignRight | Qt::AlignBottom );
        RvalueMarkers[i]->setLinePen( QColor(255, 170, 0), 1.5, Qt::DashDotLine );
        RvalueMarkers[i]->setLabel(tr("R值"));
        RvalueMarkers[i]->attach( ui->clotPlot );

        //K值标记
        KvalueMarkers.append(new QwtPlotMarker());
        KvalueMarkers[i]->setLineStyle(QwtPlotMarker::VLine);
        KvalueMarkers[i]->setLabelAlignment( Qt::AlignRight | Qt::AlignBottom );
        KvalueMarkers[i]->setLinePen( QColor(85, 255, 127), 1.5, Qt::DashDotLine );
        KvalueMarkers[i]->setLabel(tr("K值"));
      //  KvalueMarkers[i]->attach( ui->clotPlot );

        //MA值标记
        MAvalueMarkers.append(new QwtPlotMarker());
        MAvalueMarkers[i]->setLineStyle(QwtPlotMarker::HLine);
        MAvalueMarkers[i]->setLabelAlignment( Qt::AlignRight | Qt::AlignBottom );
        MAvalueMarkers[i]->setLinePen( QColor(170, 0, 127), 1.5, Qt::DashDotLine );
        MAvalueMarkers[i]->setLabel(tr("MA值"));
        MAvalueMarkers[i]->attach( ui->clotPlot );

        AngleCurves.append(new QwtPlotCurve());
        AngleCurves[i]->setPen(QColor(0, 85, 255),1.5,Qt::DashDotLine );
        AngleCurves[i]->attach(ui->clotPlot);

        preRValuePoint.append(QPointF(0,0));
        RValuePoint.append(QPointF(0,0));
        KValuePoint.append(QPointF(0,0));
        MAValuePoint.append(QPointF(0,0));
        AngleValuePoint.append(QPointF(0,0));


    }


    // 设置Y轴的坐标轴设置
    ui->plot->setAxis(ui->cbAutoScale->isChecked(),
                      ui->spYmin->value(), ui->spYmax->value());

    ui->plot_2->setAxis(ui->cbAutoScale->isChecked(),
                      ui->spYmin->value(), ui->spYmax->value());

    ui->plot_3->setAxis(ui->cbAutoScale->isChecked(),
                      ui->spYmin->value(), ui->spYmax->value());

    ui->plot_4->setAxis(ui->cbAutoScale->isChecked(),
                      ui->spYmin->value(), ui->spYmax->value());

    ui->BaseandElasticPlot->setAxis(ui->cbAutoScale->isChecked(),
                      ui->spYmin->value(), ui->spYmax->value());

    ui->clotPlot->setAxis(ui->cbAutoScale->isChecked(),
                      ui->spYmin->value(), ui->spYmax->value());

    // 初始化Y轴量程的配置
    for (int nbits = 8; nbits <= 24; nbits++) // 有符号位的配置
    {
        int rmax = pow(2, nbits-1)-1;
        int rmin = -rmax-1;
        Range r = {double(rmin),  double(rmax)};
        ui->cbRangePresets->addItem(
            QString().sprintf("Signed %d bits %d to +%d", nbits, rmin, rmax),
            QVariant::fromValue(r));
    }
    for (int nbits = 8; nbits <= 24; nbits++) // 无符号位的配置
    {
        int rmax = pow(2, nbits)-1;
        ui->cbRangePresets->addItem(
            QString().sprintf("Unsigned %d bits %d to +%d", nbits, 0, rmax),
            QVariant::fromValue(Range{0, double(rmax)}));
    }
    ui->cbRangePresets->addItem("-1 to +1", QVariant::fromValue(Range{-1, +1}));
    ui->cbRangePresets->addItem("0 to +1", QVariant::fromValue(Range{0, +1}));
    ui->cbRangePresets->addItem("-100 to +100", QVariant::fromValue(Range{-100, +100}));
    ui->cbRangePresets->addItem("0 to +100", QVariant::fromValue(Range{0, +100}));

    QObject::connect(ui->cbRangePresets, SIGNAL(activated(int)),
                     this, SLOT(onRangeSelected()));

    // 初始化每秒接收字节的显示
    spsLabel.setText("0sps");
    spsLabel.setToolTip("samples per second (total of all channels)");
    ui->statusBar->addPermanentWidget(&spsLabel);
    QObject::connect(&dataFormatPanel,
                     &DataFormatPanel::samplesPerSecondChanged,
                     this, &MainWindow::onSpsChanged);



    // 初始化示例
    QObject::connect(ui->actionDemoMode, &QAction::toggled,
                     this, &MainWindow::enableDemo);

    {   // 初始化示例的标志
        QwtText demoText(" DEMO RUNNING ");
        demoText.setColor(QColor("white"));
        demoText.setBackgroundBrush(Qt::darkRed);
        demoText.setBorderRadius(4);
        demoText.setRenderFlags(Qt::AlignLeft | Qt::AlignTop);
        demoIndicator.setText(demoText);
        demoIndicator.hide();
        demoIndicator.attach(ui->plot);
    }

    //基线及弹力值测试

    QObject::connect(&maintenanceDlg,&MaintenanceDlg::starteTest,
                     &dataFormatPanel,&DataFormatPanel::startBaseTest);
    QObject::connect(&dataFormatPanel,&DataFormatPanel::sendMaxMinValue,
                     &maintenanceDlg,&MaintenanceDlg::onSetBaseValue);
    QObject::connect(&dataFormatPanel,&DataFormatPanel::sendBaseMaxValue,
                     this,&setBaseMaxValue);
    QObject::connect(&dataFormatPanel,&DataFormatPanel::sendBaseMinValue,
                     this,&setBaseMinValue);
    QObject::connect(&dataFormatPanel,&DataFormatPanel::sendElasticMaxMinValue,
                     this,&setElasticMaxMinValue);
}

MainWindow::~MainWindow()
{
    for (auto curve : curves){
        // 释放所有曲线的内存
        delete curve;
    }
    for (auto basecurve: baseAndElaticscurves){
        // 释放所有曲线的内存
        delete basecurve;
    }
    for (auto maxclotcurve : maxClotCurves){
        // 释放所有曲线的内存
        delete maxclotcurve;
    }
    for (auto minclotcurve : minClotCurves){
        // 释放所有曲线的内存
        delete minclotcurve;
    }

    for(auto rvaluemarkers : RvalueMarkers){
        delete rvaluemarkers;
    }

    for(auto kvaluemarkers : KvalueMarkers){
        delete kvaluemarkers;
    }

    for(auto mavaluemarkers : MAvalueMarkers){
        delete mavaluemarkers;
    }

    for(auto anglecurve :AngleCurves){
        delete anglecurve;
    }


    if (serialPort.isOpen()){
        serialPort.close();
    }


    delete ui;
    ui = NULL;
}

void MainWindow::setupAboutDialog()
{
    Ui_AboutDialog uiAboutDialog;
    uiAboutDialog.setupUi(&aboutDialog);

    QObject::connect(uiAboutDialog.pbAboutQt, &QPushButton::clicked,
                     [](){ QApplication::aboutQt();});

}

void MainWindow::onPortToggled(bool open)
{
    // 如果端口打开就使能关闭示例
    if (open && isDemoRunning()) enableDemo(false);
    ui->actionDemoMode->setEnabled(!open);
}

void MainWindow::onPortError(QSerialPort::SerialPortError error)
{
    switch(error)
    {
        case QSerialPort::NoError :
            break;
        case QSerialPort::ResourceError :
            qWarning() << "Port error: resource unavaliable; most likely device removed.";
            if (serialPort.isOpen())
            {
                qWarning() << "Closing port on resource error: " << serialPort.portName();
                portControl.togglePort();
            }
            portControl.loadPortList();
            break;
        case QSerialPort::DeviceNotFoundError:
            qCritical() << "Device doesn't exists: " << serialPort.portName();
            break;
        case QSerialPort::PermissionError:
            qCritical() << "Permission denied. Either you don't have \
required privileges or device is already opened by another process.";
            break;
        case QSerialPort::OpenError:
            qWarning() << "Device is already opened!";
            break;
        case QSerialPort::NotOpenError:
            qCritical() << "Device is not open!";
            break;
        case QSerialPort::ParityError:
            qCritical() << "Parity error detected.";
            break;
        case QSerialPort::FramingError:
            qCritical() << "Framing error detected.";
            break;
        case QSerialPort::BreakConditionError:
            qCritical() << "Break condition is detected.";
            break;
        case QSerialPort::WriteError:
            qCritical() << "An error occurred while writing data.";
            break;
        case QSerialPort::ReadError:
            qCritical() << "An error occurred while reading data.";
            break;
        case QSerialPort::UnsupportedOperationError:
            qCritical() << "Operation is not supported.";
            break;
        case QSerialPort::TimeoutError:
            qCritical() << "A timeout error occurred.";
            break;
        case QSerialPort::UnknownError:
            qCritical() << "Unknown error!";
            break;
        default:
            qCritical() << "Unhandled port error: " << error;
            break;
    }
}

void MainWindow::clearPlot()
{
    for (int ci = 0; ci < channelBuffers.size(); ci++)
    {
        channelBuffers[ci]->clear();
        baseElasticBuffers[ci]->clear();
        maxclotingBuffers[ci]->clear();
        minclotingBuffers[ci]->clear();
        DataFormatPanel::count[ci] = 0;
        DataFormatPanel::clotingCount[ci] = 0;
        DataFormatPanel::isStartDetectFlag[ci] = 0;
        DataFormatPanel::isStopDetectFlag[ci] = 0;
        preRValuePoint[ci]=QPointF(0,0);
        RValuePoint[ci]=QPointF(0,0);
        KValuePoint[ci]=QPointF(0,0);
        MAValuePoint[ci]=QPointF(0,0);
        AngleValuePoint[ci]=QPointF(0,0);
        AngleCurves[ci]->detach();
    }
    ui->lbRvalue->setText("*");
    ui->lbKvalue->setText("N/A");
    ui->lbAngleValue->setText("*");
    ui->lbMAValue->setText("*");

    ui->plot->replot();
    ui->plot_2->replot();
    ui->plot_3->replot();
    ui->plot_4->replot();
    ui->BaseandElasticPlot->replot();
    ui->clotPlot->replot();
}

void MainWindow::onNumOfSamplesChanged(int value)
{
    numOfSamples = value;

    for (int ci = 0; ci < channelBuffers.size(); ci++)
    {
        channelBuffers[ci]->resize(numOfSamples);
        baseElasticBuffers[ci]->resize(numOfSamples);
        maxclotingBuffers[ci]->resize(numOfSamples);
        minclotingBuffers[ci]->resize(numOfSamples);
    }
   // ui->plot->setAxis(QwtPlot::xBottom,0,value);
    ui->plot->replot();
    ui->plot_2->replot();
    ui->plot_3->replot();
    ui->plot_4->replot();
    ui->BaseandElasticPlot->replot();
    ui->clotPlot->replot();
}

void MainWindow::onNumOfChannelsChanged(unsigned value)
{
    unsigned int oldNum = channelBuffers.size();
    unsigned numOfChannels = value;

    if (numOfChannels > oldNum)
    {
        // 如果通道数比原来大，就增加通道数
        for (unsigned int i = 0; i < numOfChannels - oldNum; i++)
        {
            //添加原始数据及曲线
            channelBuffers.append(new SignalData());
            curves.append(new QwtPlotCurve());
            curves.last()->setSamples(channelBuffers.last());
            curves.last()->setPen(Plot::makeColor(curves.length()-1));
            switch ((curves.length()-1)%4) {
            case 0:
                curves.last()->attach(ui->plot);
                break;
            case 1:
                curves.last()->attach(ui->plot_2);
                break;
            case 2:
                curves.last()->attach(ui->plot_3);
                break;
            case 3:
                curves.last()->attach(ui->plot_4);
                break;
            default:
                break;
            }


            baseElasticBuffers.append(new SignalData());
            baseAndElaticscurves.append(new QwtPlotCurve());
            baseAndElaticscurves.last()->setSamples(baseElasticBuffers.last());
            baseAndElaticscurves.last()->setPen(Plot::makeColor(baseAndElaticscurves.length()-1));
            baseAndElaticscurves.last()->attach(ui->BaseandElasticPlot);


            //添加最大值轮廓曲线和数据
            maxclotingBuffers.append(new SignalData());
            maxClotCurves.append(new QwtPlotCurve());
            maxClotCurves.last()->setSamples(maxclotingBuffers.last());
            maxClotCurves.last()->setPen(Plot::makeColor(maxClotCurves.length()-1));
            maxClotCurves.last()->attach(ui->clotPlot);


            //添加最小值轮廓曲线和数据
            minclotingBuffers.append(new SignalData());
            minClotCurves.append(new QwtPlotCurve());
            minClotCurves.last()->setSamples(minclotingBuffers.last());
            minClotCurves.last()->setPen(Plot::makeColor(minClotCurves.length()-1));         
            minClotCurves.last()->attach(ui->clotPlot);


            //添加R值标签
            RvalueMarkers.append(new QwtPlotMarker());
            RvalueMarkers.last()->setLineStyle(QwtPlotMarker::VLine);
            RvalueMarkers.last()->setLabelAlignment( Qt::AlignRight | Qt::AlignBottom );
            RvalueMarkers.last()->setLinePen( QColor(255, 170, 0), 1, Qt::DashDotLine );
            RvalueMarkers.last()->setLabel(tr("R值"));
            RvalueMarkers.last()->attach( ui->clotPlot );

            //添加K值标签
            KvalueMarkers.append(new QwtPlotMarker());
            KvalueMarkers.last()->setLineStyle(QwtPlotMarker::VLine);
            KvalueMarkers.last()->setLabelAlignment( Qt::AlignRight | Qt::AlignBottom );
            KvalueMarkers.last()->setLinePen( QColor(85, 255, 127), 1, Qt::DashDotLine );
            KvalueMarkers.last()->setLabel(tr("K值"));
            KvalueMarkers.last()->attach( ui->clotPlot );

            //添加MA值标签
            MAvalueMarkers.append(new QwtPlotMarker());
            MAvalueMarkers.last()->setLineStyle(QwtPlotMarker::HLine);
            MAvalueMarkers.last()->setLabelAlignment( Qt::AlignRight | Qt::AlignBottom );
            MAvalueMarkers.last()->setLinePen( QColor(170, 0, 127), 1, Qt::DashDotLine );
            MAvalueMarkers.last()->setLabel(tr("MA值"));
            MAvalueMarkers.last()->attach( ui->clotPlot );


            AngleCurves.append(new QwtPlotCurve());
            AngleCurves.last()->setPen(QColor(0, 85, 255),1.0,Qt::DashDotLine);
            AngleCurves.last()->attach(ui->clotPlot);




        }
    }
    else if(numOfChannels < oldNum)
    {
        // 否则就移除通道数
        for (unsigned int i = 0; i < oldNum - numOfChannels; i++)
        {
            // 并且释放曲线的数据
            delete curves.takeLast();
            delete baseAndElaticscurves.takeLast();
            delete maxClotCurves.takeLast();
            delete minClotCurves.takeLast();
            channelBuffers.removeLast();
            baseElasticBuffers.removeLast();
            maxclotingBuffers.removeLast();
            minclotingBuffers.removeLast();
            delete RvalueMarkers.takeLast();
            delete KvalueMarkers.takeLast();
            delete MAvalueMarkers.takeLast();
            delete AngleCurves.takeLast();
        }
    }
}

void MainWindow::onAutoScaleChecked(bool checked)
{
    if (checked)
    {
        ui->plot->setAxis(true);
        ui->plot_2->setAxis(true);
        ui->plot_3->setAxis(true);
        ui->plot_4->setAxis(true);
        ui->BaseandElasticPlot->setAxis(true);
        ui->clotPlot->setAxis(true);
        ui->lYmin->setEnabled(false);
        ui->lYmax->setEnabled(false);
        ui->spYmin->setEnabled(false);
        ui->spYmax->setEnabled(false);
    }
    else
    {
        ui->lYmin->setEnabled(true);
        ui->lYmax->setEnabled(true);
        ui->spYmin->setEnabled(true);
        ui->spYmax->setEnabled(true);

        ui->plot->setAxis(false,  ui->spYmin->value(), ui->spYmax->value());
        ui->plot_2->setAxis(false,  ui->spYmin->value(), ui->spYmax->value());
        ui->plot_3->setAxis(false,  ui->spYmin->value(), ui->spYmax->value());
        ui->plot_4->setAxis(false,  ui->spYmin->value(), ui->spYmax->value());
        ui->BaseandElasticPlot->setAxis(false,  ui->spYmin->value(), ui->spYmax->value());
        ui->clotPlot->setAxis(false,  ui->spYmin->value(), ui->spYmax->value());
    }
}

void MainWindow::onYScaleChanged()
{
    ui->plot->setAxis(false,  ui->spYmin->value(), ui->spYmax->value());
    ui->plot_2->setAxis(false,  ui->spYmin->value(), ui->spYmax->value());
    ui->plot_3->setAxis(false,  ui->spYmin->value(), ui->spYmax->value());
    ui->plot_4->setAxis(false,  ui->spYmin->value(), ui->spYmax->value());
    ui->BaseandElasticPlot->setAxis(false,  ui->spYmin->value(), ui->spYmax->value());
    ui->clotPlot->setAxis(false,  ui->spYmin->value(), ui->spYmax->value());
}

void MainWindow::onRangeSelected()
{
    Range r = ui->cbRangePresets->currentData().value<Range>();
    ui->spYmin->setValue(r.rmin);
    ui->spYmax->setValue(r.rmax);
    ui->cbAutoScale->setChecked(false);
}

void MainWindow::onSpsChanged(unsigned sps)
{
    spsLabel.setText(QString::number(sps) + "sps");
}

bool MainWindow::isDemoRunning()
{
    return ui->actionDemoMode->isChecked();
}

void MainWindow::enableDemo(bool enabled)
{
    if (enabled)
    {
        if (!serialPort.isOpen())
        {
            dataFormatPanel.enableDemo(true);
            ui->actionDemoMode->setChecked(true);
            demoIndicator.show();
            ui->plot->replot();
            ui->plot_2->replot();
            ui->plot_3->replot();
            ui->plot_4->replot();

        }
        else
        {
            ui->actionDemoMode->setChecked(false);
        }
    }
    else
    {
        dataFormatPanel.enableDemo(false);
        ui->actionDemoMode->setChecked(false);
        demoIndicator.hide();
        ui->plot->replot();
        ui->plot_2->replot();
        ui->plot_3->replot();
        ui->plot_4->replot();
    }
}

void MainWindow::onExportCsv()
{
    bool wasPaused = ui->actionPause->isChecked();
    ui->actionPause->setChecked(true); // 先使能暂停按钮

    QString fileName = QFileDialog::getSaveFileName(this, tr("Export CSV File"));

    if (fileName.isNull())  // 判断文件名是否有效
    {
        ui->actionPause->setChecked(wasPaused);
    }
    else
    {
        QFile file(fileName);
        if (file.open(QIODevice::WriteOnly | QIODevice::Text))
        {
            QTextStream fileStream(&file);

            unsigned numOfChannels = channelBuffers.size();
            for (unsigned int ci = 0; ci < numOfChannels; ci++)
            {
                fileStream << "Channel " << ci;
                if (ci != numOfChannels-1) fileStream << ",";
            }
            fileStream << '\n';
            numOfSamples = channelBuffers.at(0)->size();
            for (unsigned int i = 0; i < numOfSamples; i++)
            {
                for (unsigned int ci = 0; ci < numOfChannels; ci++)
                {
                    fileStream << channelBuffers[ci]->sample(i).y();
                    if (ci != numOfChannels-1) fileStream << ",";
                }
                fileStream << '\n';
            }
        }
        else
        {
            qCritical() << "File open error during export: " << file.error();
        }
    }
}

void MainWindow::messageHandler(QtMsgType type,
                                const QMessageLogContext &context,
                                const QString &msg)
{
    QString logString;

    switch (type)
    {
        case QtDebugMsg:
            logString = "[Debug] " + msg;
            break;
        case QtWarningMsg:
            logString = "[Warning] " + msg;
            break;
        case QtCriticalMsg:
            logString = "[Error] " + msg;
            break;
        case QtFatalMsg:
            logString = "[Fatal] " + msg;
            break;
    }

    if (ui != NULL) ui->ptLog->appendPlainText(logString);
    std::cerr << logString.toStdString() << std::endl;

    if (type != QtDebugMsg && ui != NULL)
    {
        ui->statusBar->showMessage(msg, 5000);
    }
}

void MainWindow::on_actionComSet_triggered()
{
    if(!isComSetOpen){
        isComSetOpen = true;
        ui->comsetFrame->show();
    }else{
        isComSetOpen = false;
        ui->comsetFrame->hide();
    }
}


#ifndef QT_NO_PRINTER
void MainWindow::on_actionPrint_triggered()
{
    QPrinter printer( QPrinter::HighResolution );

    QString docName = ui->plot->title().text();
    if ( !docName.isEmpty() )
    {
        docName.replace ( QRegExp ( QString::fromLatin1 ( "\n" ) ), tr ( " -- " ) );
        printer.setDocName ( docName );
    }

    printer.setCreator(tr("TEG example") );
    printer.setOrientation( QPrinter::Landscape );

    QPrintDialog dialog( &printer );
    if ( dialog.exec() )
    {
        QwtPlotRenderer renderer;

        if ( printer.colorMode() == QPrinter::GrayScale )
        {
            renderer.setDiscardFlag( QwtPlotRenderer::DiscardBackground );
            renderer.setDiscardFlag( QwtPlotRenderer::DiscardCanvasBackground );
            renderer.setDiscardFlag( QwtPlotRenderer::DiscardCanvasFrame );
            renderer.setLayoutFlag( QwtPlotRenderer::FrameWithScales );
        }

        renderer.renderTo( ui->plot, printer );
    }
}
#endif

void MainWindow::on_actionReport_triggered()
{

    QwtPlotRenderer renderer;
    renderer.exportTo( ui->clotPlot, "teg.pdf" );
}

void MainWindow::on_action_Preset_triggered()
{
    PresetDlg presetDlg;
    if(presetDlg.exec()==QDialog::Accepted)
    {

    }

}


void MainWindow::on_pbHideComFrm_clicked()
{
    ui->comsetFrame->hide();
    isComSetOpen = false;
}

//设置R值的槽函数
void MainWindow::onSetRvalue(unsigned channel, QPointF preRvaluePoint,QPointF RvaluePoint)
{
    preRValuePoint[channel] = preRvaluePoint;
    RvalueMarkers[channel]->setValue(RvaluePoint);
    double value = RvaluePoint.rx();
    QString strRvalue = QString::number(value/60,'f',2);
    if(channel==0) ui->lbRvalue->setText(strRvalue);
    if(channel==1) ui->lbRvalue_2->setText(strRvalue);
    if(channel==2) ui->lbRvalue_3->setText(strRvalue);
    RValuePoint[channel] = RvaluePoint;
}


//设置K值得槽函数
void MainWindow::onSetKvalue(unsigned channel, QPointF KvaluePoint)
{
    KvalueMarkers[channel]->setValue(KvaluePoint);
    KvalueMarkers[channel]->attach(ui->clotPlot);
    double value;
    value = KvaluePoint.rx()-RValuePoint[channel].rx();
    QString strKvalue = QString::number(value/60,'f',2);
    if(channel==0) ui->lbKvalue->setText(strKvalue);;
    if(channel==1) ui->lbKvalue_2->setText(strKvalue);
    if(channel==2) ui->lbKvalue_3->setText(strKvalue);

    KValuePoint[channel] = KvaluePoint;
}

void MainWindow::onSetMAvalue(unsigned channel, QPointF MAvaluePoint, double baselineValue)
{
    MAvalueMarkers[channel]->setValue(MAvaluePoint);
    double value;
    value = MAvaluePoint.ry();
    QString strMAvalue = QString::number((baselineValue-value)*0.047,'f',2);
    if(channel==0) ui->lbMAValue->setText(strMAvalue);;
    if(channel==1) ui->lbMAValue_2->setText(strMAvalue);
    if(channel==2) ui->lbMAValue_3->setText(strMAvalue);

    MAValuePoint[channel] = MAvaluePoint;
}

void MainWindow::onSetAnglevalue(unsigned channel, QPointF AnglevaluePoint, QPointF lineParaPoint)
{
    double tanvalue = ((AnglevaluePoint.ry()-preRValuePoint[channel].ry())*0.047)/((AnglevaluePoint.rx()-preRValuePoint[channel].rx())*4/60);
    double arctanvalue = (180*qAtan(tanvalue))/M_PI;
    qDebug()<<"arctanvalue"<<arctanvalue;
    QString strAnglevalue = QString::number(arctanvalue,'f',2);
    double xvalue[3] = {preRValuePoint[channel].x(),AnglevaluePoint.x(),getlineX(lineParaPoint,4000)};
    double yvalue[3] = {preRValuePoint[channel].y(),AnglevaluePoint.y(),4000};
    qDebug()<<xvalue[0]<<" "<<yvalue[0];
    qDebug()<<xvalue[1]<<" "<<yvalue[1];
    qDebug()<<xvalue[2]<<" "<<yvalue[2];
    AngleCurves[channel]->setSamples(xvalue,yvalue,3);
  //  ui->clotPlot->replot();
    if(channel==0) ui->lbAngleValue->setText(strAnglevalue);;
    if(channel==1) ui->lbAngleValue_2->setText(strAnglevalue);
    if(channel==2) ui->lbAngleValue_3->setText(strAnglevalue);
    AngleValuePoint[channel] = AnglevaluePoint;
}

double MainWindow::getlineX(QPointF lineSlope, double yValue)
{
    double  xValue;
    xValue = (yValue-lineSlope.ry())/lineSlope.rx();
    return xValue;
}

void MainWindow::on_actionStop_triggered()
{
    emit ui->actionStart->triggered(false);
    ui->actionStart->setChecked(false);
}

void MainWindow::on_action_maintance_triggered()
{

}

void MainWindow::setBaseMaxValue(unsigned channel, double maxValue, double minValue,double startTimes)
{
    double maxDiffValue = maxValue-minValue;
    startTimeArray[channel]=startTimes;
    QString strmaxDiffValue = QString::number(maxDiffValue,'f',0);
    ui->baseAdjtableWidget->setItem(channel,1,new QTableWidgetItem(strmaxDiffValue));
}

void MainWindow::setBaseMinValue(unsigned channel, double maxValue, double minValue,double stopTimes)
{
    double minDiffValue = maxValue-minValue;
    double stepTimes = stopTimes-startTimeArray[channel];
    QString strminDiffValue = QString::number(minDiffValue,'f',0);
    QString strStepTimeValue = QString::number(stepTimes,'f',0);
    ui->baseAdjtableWidget->setItem(channel,2,new QTableWidgetItem(strminDiffValue));
    ui->baseAdjtableWidget->setItem(channel,3,new QTableWidgetItem(strStepTimeValue));
}

void MainWindow::setElasticMaxMinValue(unsigned channel, double maxValue, double minValue)
{
    QString minStr = QString::number(minValue,'f',0);
    ui->elastictableWidget->setItem(channel,1,new QTableWidgetItem(minStr));

    QString maxStr = QString::number(maxValue,'f',0);
    ui->elastictableWidget->setItem(channel,2,new QTableWidgetItem(maxStr));

    QString diffStr = QString::number(maxValue-minValue,'f',0);
    ui->elastictableWidget->setItem(channel,3,new QTableWidgetItem(diffStr));
}

void MainWindow::on_actionBarCode_triggered()
{
    barcodeDlg.setWindowTitle(tr("样品录入信息"));
    if(barcodeDlg.exec()==QDialog::Accepted){

    }
}

void MainWindow::on_actionPatient_triggered()
{
    resultshowDlg.setWindowTitle(tr("样品检测结果"));
    if(resultshowDlg.exec()==QDialog::Accepted){

    }
}
