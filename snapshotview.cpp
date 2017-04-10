
#include <QSaveFile>

#include "snapshotview.h"
#include "ui_snapshotview.h"

SnapshotView::SnapshotView(QWidget *parent, Snapshot* snapshot) :
    QMainWindow(parent),
    ui(new Ui::SnapshotView),
    renameDialog(this)
{
    _snapshot = snapshot;

    ui->setupUi(this);
    ui->menuSnapshot->insertAction(ui->actionClose, snapshot->deleteAction());
    this->setWindowTitle(snapshot->name());
    ui->plot->setAxisScale(QwtPlot::yLeft,0,4096);

    unsigned numOfChannels = snapshot->data.size();

    for (unsigned ci = 0; ci < numOfChannels; ci++)
    {
        curves.append(new QwtPlotCurve);
        curves[ci]->setSamples(snapshot->data[ci]);
        curves[ci]->setPen(Plot::makeColor(ci));
        curves[ci]->attach(ui->plot);
//        curve->hide();
    }

//    magCurve.append(new QwtPlotCurve);
   // magCurve.setSamples(snapshot->data[ci]);
    magCurve.setPen(Qt::blue);
    magCurve.setCurveAttribute(QwtPlotCurve::Fitted);
    magCurve.attach(ui->plot);

    curveFitter = new QwtSplineCurveFitter();
    curveFitter->setFitMode(QwtSplineCurveFitter::ParametricSpline);
    curveFitter->setSplineSize(20);


    Arc_Teg_Time_Curve.setPen(Qt::blue,2.0,Qt::SolidLine);
    Arc_Teg_Time_Curve.setCurveAttribute(QwtPlotCurve::Fitted);
    Arc_Teg_Time_Curve.setCurveFitter(curveFitter);

    Arc_Teg_Time_Curve.attach(ui->plot);

    Teg_Time_Curve.setPen(Qt::blue,2.0,Qt::SolidLine);
    Teg_Time_Curve.setCurveAttribute(QwtPlotCurve::Fitted);
    Teg_Time_Curve.setCurveFitter(curveFitter);

    Teg_Time_Curve.attach(ui->plot);

    RMAGvalueMarkers.setLineStyle(QwtPlotMarker::VLine);
    RMAGvalueMarkers.setLabelAlignment( Qt::AlignRight | Qt::AlignBottom );
    RMAGvalueMarkers.setLinePen( QColor(255, 170, 0), 2, Qt::DashDotLine );
    RMAGvalueMarkers.setLabel(tr("R值"));
    RMAGvalueMarkers.attach( ui->plot );

    KMAGvalueMarkers.setLineStyle( QwtPlotMarker::VLine );
    KMAGvalueMarkers.setLabelAlignment( Qt::AlignRight | Qt::AlignBottom );
    KMAGvalueMarkers.setLinePen( QColor(85, 255, 127), 1.5, Qt::DashDotLine );
    KMAGvalueMarkers.setLabel(tr("K值"));
   // KvalueMarkers.attach( ui->tegCurvePlot );

    MAMAGvalueMarkers.setLineStyle(QwtPlotMarker::HLine);
    MAMAGvalueMarkers.setLabelAlignment( Qt::AlignRight | Qt::AlignBottom );
    MAMAGvalueMarkers.setLinePen( QColor(170, 0, 127), 1.5, Qt::DashDotLine );
    MAMAGvalueMarkers.setLabel(tr("MA值"));
    MAMAGvalueMarkers.attach( ui->plot );

    AngleMAGCruve.setStyle( QwtPlotCurve::Lines );
    AngleMAGCruve.setPen( QPen(QColor(0, 85, 255),1.5,Qt::DashDotLine));
    AngleMAGCruve.setRenderHint( QwtPlotItem::RenderAntialiased, true );
    AngleMAGCruve.setPaintAttribute( QwtPlotCurve::FilterPoints, true );
    AngleMAGCruve.attach( ui->plot );

    renameDialog.setWindowTitle("Rename Snapshot");
    renameDialog.setLabelText("Enter new name:");
    connect(ui->actionRename, &QAction::triggered,
            this, &SnapshotView::showRenameDialog);

    connect(ui->actionExport, &QAction::triggered,
            this, &SnapshotView::save);

    for (auto a : ui->plot->menuActions())
    {
        ui->menuView->addAction(a);
    }
}

SnapshotView::~SnapshotView()
{
    for (auto curve : curves)
    {
        delete curve;
    }
    delete ui;
}

void SnapshotView::closeEvent(QCloseEvent *event)
{
    QMainWindow::closeEvent(event);
    emit closed();
}

void SnapshotView::showRenameDialog()
{
    renameDialog.setTextValue(_snapshot->name());
    renameDialog.open(this, SLOT(renameSnapshot(QString)));
}

void SnapshotView::renameSnapshot(QString name)
{
    _snapshot->setName(name);
    setWindowTitle(name);
}

void SnapshotView::save()
{
    QString fileName = QFileDialog::getSaveFileName(this, tr("Export CSV File"));

    if (fileName.isNull()) return; // user canceled

    // TODO: remove code duplication (MainWindow::onExportCsv)
    QSaveFile file(fileName);

    if (file.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        QTextStream fileStream(&file);

        numOfChannels = _snapshot->data.size();
        numOfSamples = _snapshot->data[0].size();

        // 输出文件头
        for (unsigned int ci = 0; ci < numOfChannels; ci++)
        {
            fileStream << "Channel " << ci;
            if (ci != numOfChannels-1) fileStream << ",";
        }
        fileStream << '\n';

        // 打印输出数组
        for (unsigned int i = 0; i < numOfSamples; i++)
        {
            for (unsigned int ci = 0; ci < numOfChannels; ci++)
            {
                fileStream << _snapshot->data[ci][i].y();
                if (ci != numOfChannels-1) fileStream << ",";
            }
            fileStream << '\n';
        }

        if (!file.commit())
        {
            qCritical() << tr("数据存储过程发生错误:") << file.error();
        }
    }
    else
    {
        qCritical() << tr("将照片数据存储过程发生错误:") << file.error();
    }
}


//对存储的数据进行凝血出来 2016/04/08

void SnapshotView::on_actionClotData_triggered(bool checked)
{
    if(checked)
    {
        if(_snapshot->data[0].isEmpty()){
              qCritical() << tr("没有可以使用的数据");
              return;
        }
        numOfChannels = _snapshot->data.size();
        numOfSamples = _snapshot->data[0].size();
        qDebug()<<numOfSamples;
        qDebug()<<numOfChannels;
        clotMaxval.resize(numOfChannels);
        clotMinval.resize(numOfChannels);
        BaseClotDataMax.resize(numOfChannels);
        BaseClotDataMin.resize(numOfChannels);
        RPoint.resize(numOfChannels);
        AnglePoint.resize(numOfChannels);
        linepara.resize(numOfChannels);
        for(unsigned int ci=0; ci < numOfChannels; ci++)
        {
            numOfSamples = _snapshot->data[ci].size();
            p_proclotmin.append(new QwtPlotCurve);
            p_proclotmin[ci]->setStyle( QwtPlotCurve::Lines );
            p_proclotmin[ci]->setPen(Plot::makeColor(ci),1.0,Qt::SolidLine);
            p_proclotmin[ci]->setRenderHint( QwtPlotItem::RenderAntialiased, true );
            p_proclotmin[ci]->setPaintAttribute( QwtPlotCurve::FilterPoints, true );
        //    p_proclotmin[ci]->setCurveFitter(curveFitter);
            p_proclotmin[ci]->attach( ui->plot );


            p_proclotmax.append(new QwtPlotCurve);
            p_proclotmax[ci]->setStyle( QwtPlotCurve::Lines );
            p_proclotmax[ci]->setPen( QPen(Qt::red,2.0,Qt::SolidLine) );
            p_proclotmax[ci]->setRenderHint( QwtPlotItem::RenderAntialiased, true );
            p_proclotmax[ci]->setPaintAttribute( QwtPlotCurve::FilterPoints, true );
            p_proclotmax[ci]->setPen(Plot::makeColor(ci),1.0,Qt::SolidLine);
           // p_proclotmax[ci]->setCurveFitter(curveFitter);
            p_proclotmax[ci]->attach( ui->plot );

            R_marker.append(new QwtPlotMarker);
            R_marker[ci]->setLineStyle( QwtPlotMarker::VLine );
            R_marker[ci]->setLabelAlignment( Qt::AlignRight | Qt::AlignBottom );
            R_marker[ci]->setLinePen( QColor(255, 170, 0), 1.5, Qt::DashDotLine );
            R_marker[ci]->setLabel(tr("R值"));
            R_marker[ci]->attach( ui->plot );

            K_marker.append(new QwtPlotMarker);
            K_marker[ci]->setLineStyle( QwtPlotMarker::VLine );
            K_marker[ci]->setLabelAlignment( Qt::AlignRight | Qt::AlignBottom );
            K_marker[ci]->setLinePen( QColor(85, 255, 127), 1.5, Qt::DashDotLine );
            K_marker[ci]->setLabel(tr("K值"));
            K_marker[ci]->attach( ui->plot );

            MA_marker.append(new QwtPlotMarker);
            MA_marker[ci]->setLineStyle( QwtPlotMarker::HLine );
            MA_marker[ci]->setLabelAlignment( Qt::AlignRight | Qt::AlignBottom );
            MA_marker[ci]->setLinePen( QColor(170, 0, 127), 1.5, Qt::DashDotLine );
            MA_marker[ci]->setLabel(tr("MA值"));
            MA_marker[ci]->attach( ui->plot );

            AngleCruve.append(new QwtPlotCurve);
            AngleCruve[ci]->setStyle( QwtPlotCurve::Lines );
            AngleCruve[ci]->setPen( QPen(QColor(0, 85, 255),1.5,Qt::DashDotLine));
            AngleCruve[ci]->setRenderHint( QwtPlotItem::RenderAntialiased, true );
            AngleCruve[ci]->setPaintAttribute( QwtPlotCurve::FilterPoints, true );
            AngleCruve[ci]->attach( ui->plot );

            int imax;
            int imin;

            int basevalue=0;
            int basesum=0;


            //clotMaxval[ci].clear();
           // clotMaxval[ci].clear();

            for(unsigned int i = 0;i < 275;i+=55)
            {
                imax=_snapshot->data[ci][i].y();
                imin=_snapshot->data[ci][i].y();
                for(unsigned int j = i; j<i+55; j++)
                {
                    if(imax<_snapshot->data[ci][j].y())
                        imax=_snapshot->data[ci][j].y();
                    if(imin>_snapshot->data[ci][j].y())
                        imin=_snapshot->data[ci][j].y();

                }
                basesum += (imax+imin)/2;

            }

            basevalue = basesum/5;
            int minpos=0;
            int minposvalus = 100;

            for(int i=0;i<55;i++)
            {
                if(minposvalus > fabs(_snapshot->data[ci][i].y()-basevalue)){
                   minposvalus = fabs(_snapshot->data[ci][i].y()-basevalue);
                   minpos = i;
                }

            }

            static int countpos = 0;



            for(int i = minpos;i< numOfSamples-55;i+=55)
            {
                countpos++;
                if(countpos == 2){
                    i=i+1;
                    countpos = 0;
                }

                imax=_snapshot->data[ci][i].y();
                imin=_snapshot->data[ci][i].y();
                int j;
                for(j = i;j<i+55;j++){
                    if(imax<_snapshot->data[ci][j].y())
                        imax=_snapshot->data[ci][j].y();
                    if(imin>_snapshot->data[ci][j].y())
                        imin=_snapshot->data[ci][j].y();
                }
                clotMaxval[ci].append(QPointF((double)j*0.180,imax));
                clotMinval[ci].append(QPointF((double)j*0.180,imin));
            }

        //    p_proclotmax->setSamples(clotMaxval[ci]);
        //    p_proclotmin->setSamples(clotMinval[ci]);
            curves[ci]->hide();
         //   ui->plot->replot();

            double baseData;
            double sum = 0;
            for(int i = 0; i < 5; i++){
                sum += (clotMaxval[ci].at(i).y()+clotMinval[ci].at(i).y())/2;
            }
            baseData = sum/5;
            int RValue = 0;
            int KValue = 0;
            int maValue = 0;
            int maIndex = 0;

            for(int i = 0; i < clotMaxval[ci].size(); i++){
                if(clotMaxval[ci].at(i).y() < (baseData+50)){
                    if(clotMaxval[ci].at(i).y() < (baseData+25)){
                        BaseClotDataMax[ci].append(QPointF(clotMaxval[ci].at(i).x(),baseData));
                        BaseClotDataMin[ci].append(QPointF(clotMinval[ci].at(i).x(),baseData));
                        RPoint[ci] = BaseClotDataMax[ci].at(i);
                      //  angle[0]= RPoint;
                    }else{
                        BaseClotDataMax[ci].append(clotMaxval[ci].at(i));
                        BaseClotDataMin[ci].append(clotMinval[ci].at(i));
                    }

                  //  qDebug()<<BaseClotDataMax.at(i);
                    RValue++;
                }else{
                    if(clotMaxval[ci].at(i).y() < (baseData+500)){
                        KValue ++ ;
                    }
                    if(clotMaxval[ci].at(i).y()> maValue){
                        maValue = clotMaxval[ci].at(i).y();
                        maIndex = i;
                    }
                    BaseClotDataMax[ci].append(clotMaxval[ci].at(i));
                    BaseClotDataMin[ci].append(clotMinval[ci].at(i));
                    linepara[ci] = linefunc(RPoint[ci],BaseClotDataMax[ci].at(i));
                    if(BaseClotDataMax[ci].at(i-1).y()<getlineY(linepara[ci],BaseClotDataMax[ci].at(i-1).x())&&
                            BaseClotDataMax[ci].at(i-2).y()<getlineY(linepara[ci],BaseClotDataMax[ci].at(i-2).x())){
                        AnglePoint[ci] = BaseClotDataMax[ci].at(i);
                        qDebug()<<"angle"<<AnglePoint[ci];
                        double tanvalue = ((AnglePoint[ci].ry()-RPoint[ci].ry())*0.04)/((AnglePoint[ci].rx()-RPoint[ci].rx())*4/60);
                        double arctanvalue = (180*qAtan(tanvalue))/M_PI;
                        qDebug()<<"arctanvalue"<<arctanvalue;
                        QString strAnglevalue = QString::number(arctanvalue,'f',2);
                        if(ci==0) ui->lbAngleValue->setText(strAnglevalue);;
                        if(ci==1) ui->lbAngleValue_2->setText(strAnglevalue);
                        if(ci==2) ui->lbAngleValue_3->setText(strAnglevalue);
                    //    ui->leAngleValue->setText(strAnglevalue);
                      //  angle[1]= AnglePoint;
                        double xvalue[3] = {RPoint[ci].x(),AnglePoint[ci].x(),getlineX(linepara[ci],4000)};
                        double yvalue[3] = {RPoint[ci].y(),AnglePoint[ci].y(),4000};
                        AngleCruve[ci]->setSamples(xvalue,yvalue,3);
                      //  ui->qwtPlot->replot();
                    }

                 //   qDebug()<<BaseClotDataMax.at(i);
                }


            }


            p_proclotmax[ci]->setSamples(BaseClotDataMax[ci]);
            p_proclotmin[ci]->setSamples(BaseClotDataMin[ci]);

            QString strMAvalue = QString::number(double(baseData-BaseClotDataMin[ci].at(maIndex).y())*0.047,'f',2);
            qDebug()<<strMAvalue;
            if(ci==0) ui->lbMAValue->setText(strMAvalue);;
            if(ci==1) ui->lbMAValue_2->setText(strMAvalue);
            if(ci==2) ui->lbMAValue_3->setText(strMAvalue);

            QString strRvalue = QString::number(double(RPoint[ci].x())/60,'f',2);
            if(ci==0) ui->lbRvalue->setText(strRvalue);
            if(ci==1) ui->lbRvalue_2->setText(strRvalue);
            if(ci==2) ui->lbRvalue_3->setText(strRvalue);
            qDebug()<<strRvalue;
            QString strKvalue = QString::number(double(BaseClotDataMax[ci].at(KValue).x())/60,'f',2);
            if(ci==0) ui->lbKvalue->setText(strKvalue);;
            if(ci==1) ui->lbKvalue_2->setText(strKvalue);
            if(ci==2) ui->lbKvalue_3->setText(strKvalue);
            qDebug()<<strKvalue;

            R_marker[ci]->setValue(BaseClotDataMax[ci].at(RValue));
            K_marker[ci]->setValue(BaseClotDataMax[ci].at(RValue+KValue));
            MA_marker[ci]->setValue(BaseClotDataMin[ci].at(maIndex));
            ui->plot->replot();
            //ui->replot();

        }



      //  ui->actionClotCurve->setChecked(true);


    }

}

QPointF SnapshotView::linefunc(QPointF point1, QPointF point2)
{
    QPointF linepara;
    linepara.setX((point1.ry()-point2.ry())/(point1.rx()-point2.rx()));
    linepara.setY(point1.ry()-(linepara.rx()*point1.rx()));
    return linepara;
}

double SnapshotView::getlineY(QPointF lineSlope, double xValue)
{
    double  yValue;
    yValue = (lineSlope.rx()*xValue)+lineSlope.ry();
    return yValue;
}

double SnapshotView::getlineX(QPointF lineSlope, double yValue)
{
    double  xValue;
    xValue = (yValue-lineSlope.ry())/lineSlope.rx();
    return xValue;
}


void SnapshotView::on_actionImportMagData_triggered()
{
    QString fileName = QFileDialog::getOpenFileName(this,"导入MAG数据");
    QFile file(fileName);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        qCritical() << "Couldn't open file: " << fileName;
        qCritical() << file.errorString();
        return;
    }
    while (!file.atEnd())
    {
        // 读取每行数据
        auto line = QString(file.readLine());
        auto split = line.split(',');


        if (split.size() != 2)
        {
            qCritical() << "Parsing error at line "
                        << ": number of columns is not consistent.";
            return;
        }

        double x = split[0].toDouble()+12;
        double y =(split[1].toDouble()*1000-2200)*10;

        m_vecMagData.append(QPointF(x,y));

    }
    magCurve.setSamples(m_vecMagData);
    ui->plot->replot();


}

void SnapshotView::on_actionMagClotCurve_triggered()
{
    if(m_vecMagData.isEmpty()) return;
  //  m_vecPhasePoint = phaseResPoint;
    m_vecMagTegData = m_vecMagData;
    unsigned numOfPoint = m_vecMagTegData.size();
    bool baseFlag = true;
    bool RValueFlag = false;
    m_vecArcMagTegData.resize(numOfPoint);
    double baseValue=m_vecMagTegData.at(0).y();

    m_vecArcMagTegData[0].setX(m_vecMagTegData.at(0).x());
    m_vecArcMagTegData[0].setY(baseValue);
    double MaxValue = 0;

    if(numOfPoint<=5){
        double sum=0;
        for(unsigned n=0; n<numOfPoint;n++){
            sum +=  m_vecMagTegData.at(0).y();
        }
        baseValue = sum/numOfPoint;
    }
    if(numOfPoint<300){
        curveFitter->setSplineSize(20);
    }else{
        curveFitter->setSplineSize(50);
    }
    if(numOfPoint>2){
        for(unsigned i = 0; i < numOfPoint-2; i++){
            if(baseFlag){
                if((m_vecMagTegData.at(i).y()-m_vecMagTegData.at(i+1).y()<=20)&&
                     (m_vecMagTegData.at(i+1).y()-m_vecMagTegData.at(i+2).y()<=20)){
                 //   baseValue = m_vecMagTegData.at(i+1).y();

                    for(unsigned n=0; n <= i+2; n++){

                        m_vecMagTegData[n].setY(baseValue);
                        m_vecArcMagTegData[n].setX( m_vecMagTegData.at(n).x());
                        m_vecArcMagTegData[n].setY(baseValue);

                        RMAGValuePoint = m_vecMagTegData[n];
                    }
                }

                if((m_vecMagTegData.at(i).y()-m_vecMagTegData.at(i+1).y()>20)&&
                        (m_vecMagTegData.at(i+1).y()-m_vecMagTegData.at(i+2).y()>20)){
                    baseFlag = false;
                    RValueFlag = true;

                }
                RMAGvalueMarkers.setValue(RMAGValuePoint);
                ui->lbRvalue_2->setText(QString::number(RMAGValuePoint.x()/60,'f',2));
            }

            if((m_vecMagTegData.at(i).y()>baseValue)&&
                    (m_vecMagTegData.at(i+1).y()>baseValue)){
                baseFlag = true;
                RValueFlag = false;
                baseValue = m_vecMagTegData.at(i+1).y();
            }

            if(RValueFlag){

                if(m_vecMagTegData.at(i+1).y()>m_vecMagTegData.at(i).y()+80){

                   // m_vecMagTegData[i+1].setY(m_vecMagTegData.at(i).y());
                }

                if(m_vecArcMagTegData[i].y()< baseValue+250){
                    KMAGvalueMarkers.attach(ui->plot);
                    KMAGValuePoint = m_vecMagTegData[i];
                    KMAGvalueMarkers.setValue(KMAGValuePoint);
                    ui->lbKvalue_2->setText(QString::number(KMAGValuePoint.x()/60,'f',2));
                }

            }

            m_vecArcMagTegData[i+1].setX(m_vecMagTegData.at(i+1).x());
            m_vecArcMagTegData[i+1].setY(2*baseValue - m_vecMagTegData.at(i+1).y());

            if(m_vecArcMagTegData[i].y() > MaxValue){
                MaxValue = m_vecArcMagTegData[i].y();
                MAMAGValuePoint = m_vecMagTegData[i];
                MAMAGvalueMarkers.setValue(MAMAGValuePoint);
                ui->lbMAValue_2->setText(QString::number(MAMAGValuePoint.x()*0.42,'f',1));
            }



        }
        m_vecMagTegData[numOfPoint-1].setY(m_vecMagTegData.at(numOfPoint-2).y());

        m_vecArcMagTegData[numOfPoint-1].setX(m_vecMagTegData.at(numOfPoint-1).x());
        m_vecArcMagTegData[numOfPoint-1].setY(2*baseValue - m_vecMagTegData.at(numOfPoint-1).y());

        for(unsigned i = 0; i < numOfPoint; i++){
             m_vecArcMagTegData[i].setY((2*baseValue - m_vecMagTegData.at(i).y())*0.75+(2048-baseValue));
             m_vecMagTegData[i].setY(m_vecMagTegData.at(i).y()*0.75+(2048-baseValue));
        }

        Arc_Teg_Time_Curve.setSamples(m_vecArcMagTegData);
        Teg_Time_Curve.setSamples(m_vecMagTegData);
        ui->plot->replot();

    }

}
