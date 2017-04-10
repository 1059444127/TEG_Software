
#include "dataformatpanel.h"
#include "ui_dataformatpanel.h"

#include <QtEndian>

#include "utils.h"
#include "floatswap.h"

int DataFormatPanel::count[10] = {0};
int DataFormatPanel::clotingCount[10] = {0};
int DataFormatPanel::isStartDetectFlag[10] ={0};
int DataFormatPanel::isStopDetectFlag[10] ={0};

DataFormatPanel::DataFormatPanel(QSerialPort* port,
                                 QList<SignalData*>* channelBuffers,
                                 QList<SignalData*>* baseElasticchannelBuffers,
                                 QList<SignalData*>* maxclotingBuffers,
                                 QList<SignalData*>* minclotingBuffers,
                                 QWidget *parent) :
    QWidget(parent),
    ui(new Ui::DataFormatPanel),
    MAvaluePoint(0,5000)
{
    ui->setupUi(this);

    serialPort = port;
    _channelBuffers = channelBuffers;
    _baseElasticchannelBuffers = baseElasticchannelBuffers;
    _maxclotingBuffers = maxclotingBuffers;
    _minclotingBuffers = minclotingBuffers;
    paused = false;
    isStartDetect = false;

    // 设置数据格式的按钮
    numberFormatButtons.addButton(ui->rbUint8,  NumberFormat_uint8);
    numberFormatButtons.addButton(ui->rbUint16, NumberFormat_uint16);
    numberFormatButtons.addButton(ui->rbUint32, NumberFormat_uint32);
    numberFormatButtons.addButton(ui->rbInt8,   NumberFormat_int8);
    numberFormatButtons.addButton(ui->rbInt16,  NumberFormat_int16);
    numberFormatButtons.addButton(ui->rbInt32,  NumberFormat_int32);
    numberFormatButtons.addButton(ui->rbFloat,  NumberFormat_float);
    numberFormatButtons.addButton(ui->rbASCII,  NumberFormat_ASCII);

    QObject::connect(
        &numberFormatButtons, SIGNAL(buttonToggled(int, bool)),
        this, SLOT(onNumberFormatButtonToggled(int, bool)));

    //初始化数据格式
    selectNumberFormat((NumberFormat) numberFormatButtons.checkedId());

    // 设置通道数的信号槽
    QObject::connect(ui->spNumOfChannels,
                     SELECT<int>::OVERLOAD_OF(&QSpinBox::valueChanged),
                     this, &DataFormatPanel::onNumOfChannelsSP);

    _numOfChannels = ui->spNumOfChannels->value();

    // 初始化每秒钟接收的字节数
    sampleCount = 0;
    QObject::connect(&spsTimer, &QTimer::timeout,
                     this, &DataFormatPanel::spsTimerTimeout);
    spsTimer.start(SPS_UPDATE_TIMEOUT * 1000);

    //初始化示例模式
    demoCount = 0;
    demoTimer.setInterval(100);
    QObject::connect(&demoTimer, &QTimer::timeout,
                     this, &DataFormatPanel::demoTimerTimeout);
}

DataFormatPanel::~DataFormatPanel()
{
    delete ui;
}

void DataFormatPanel::onNumberFormatButtonToggled(int numberFormatId,
                                                  bool checked)
{
    if (checked) selectNumberFormat((NumberFormat) numberFormatId);
}

void DataFormatPanel::selectNumberFormat(NumberFormat numberFormatId)
{
    numberFormat = numberFormatId;

    switch(numberFormat)
    {
        case NumberFormat_uint8:
            sampleSize = 1;
            readSample = &DataFormatPanel::readSampleAs<quint8>;
            break;
        case NumberFormat_int8:
            sampleSize = 1;
            readSample = &DataFormatPanel::readSampleAs<qint8>;
            break;
        case NumberFormat_uint16:
            sampleSize = 2;
            readSample = &DataFormatPanel::readSampleAs<quint16>;
            break;
        case NumberFormat_int16:
            sampleSize = 2;
            readSample = &DataFormatPanel::readSampleAs<qint16>;
            break;
        case NumberFormat_uint32:
            sampleSize = 4;
            readSample = &DataFormatPanel::readSampleAs<quint32>;
            break;
        case NumberFormat_int32:
            sampleSize = 4;
            readSample = &DataFormatPanel::readSampleAs<qint32>;
            break;
        case NumberFormat_float:
            sampleSize = 4;
            readSample = &DataFormatPanel::readSampleAs<float>;
            break;
        case NumberFormat_ASCII:
            sampleSize = 0;
            readSample = NULL;
            break;
    }

    if (numberFormat == NumberFormat_ASCII)
    {
        QObject::disconnect(serialPort, &QSerialPort::readyRead, 0, 0);
        QObject::connect(this->serialPort, &QSerialPort::readyRead,
                         this, &DataFormatPanel::onDataReadyASCII);
    }
    else
    {
        QObject::disconnect(serialPort, &QSerialPort::readyRead, 0, 0);
        QObject::connect(serialPort, &QSerialPort::readyRead,
                         this, &DataFormatPanel::onDataReady);
    }

    emit skipByteEnabledChanged(skipByteEnabled());
}

bool DataFormatPanel::skipByteEnabled()
{
    return numberFormat != NumberFormat_ASCII;
}

unsigned DataFormatPanel::numOfChannels()
{
    return _numOfChannels;
}

void DataFormatPanel::onNumOfChannelsSP(int value)
{
    _numOfChannels = value;
    emit numOfChannelsChanged(value);
}

void DataFormatPanel::requestSkipByte()
{
    skipByteRequested = true;
}

void DataFormatPanel::startBaseTest(bool started)
{
    startBaseTested = started;
}

void DataFormatPanel::pause(bool enabled)
{
    paused = enabled;
}

void DataFormatPanel::enableDemo(bool enabled)
{
    if (enabled)
    {
        demoTimer.start();
    }
    else
    {
        demoTimer.stop();
    }
}

void DataFormatPanel::spsTimerTimeout()
{
    unsigned currentSps = _samplesPerSecond;
    _samplesPerSecond = sampleCount/SPS_UPDATE_TIMEOUT;
    if (currentSps != _samplesPerSecond)
    {
        emit samplesPerSecondChanged(_samplesPerSecond);
    }
    sampleCount = 0;
}


void DataFormatPanel::demoTimerTimeout()
{
    const double period = 100;
    demoCount++;
    if (demoCount >= 100) demoCount = 0;

    if (!paused)
    {
        for (unsigned ci = 0; ci < _numOfChannels; ci++)
        {
            // 计算示例的数据
            double value = 4*sin(2*M_PI*double((ci+1)*demoCount)/period)/((2*(ci+1))*M_PI);
            addChannelData(ci, &value, 1);
        }
        emit dataAdded();
    }
}

void DataFormatPanel::onDataReady()
{
    // 一次采集数据的一个包保护的数据格式是 {通道1, 通道2...}
    int packageSize = sampleSize * _numOfChannels ;
    int bytesAvailable = serialPort->bytesAvailable();
    int numOfPackagesToRead =
        (bytesAvailable - (bytesAvailable % packageSize)) / packageSize;

    if (paused)
    {
        // 如果暂停就放弃接收数据
        serialPort->read(numOfPackagesToRead*packageSize);
        return;
    }

    if (bytesAvailable > 0 && skipByteRequested)
    {
        serialPort->read(1);
        skipByteRequested = false;
        bytesAvailable--;
    }

    if (bytesAvailable%(packageSize*36) != 0) return;

    double* channelSamples = new double[numOfPackagesToRead*_numOfChannels];

    for (int i = 0; i < numOfPackagesToRead; i++)
    {
        for (unsigned int ci = 0; ci < _numOfChannels; ci++)
        {
            // channelSamples[ci].replace(i, (this->*readSample)());
            channelSamples[ci*numOfPackagesToRead+i] = (this->*readSample)();
        }
    }


    for (unsigned int ci = 0; ci < _numOfChannels; ci++)
    {
        addChannelData(ci,
                       channelSamples + ci*numOfPackagesToRead,
                       numOfPackagesToRead);
    }
    emit dataAdded();

    delete channelSamples;
}

void DataFormatPanel::onDataReadyASCII()
{
    while(serialPort->canReadLine())
    {
        QByteArray line = serialPort->readLine();

        // 如果暂停就不接受数据
        if (paused)
        {
            return;
        }

        line = line.trimmed();
        auto separatedValues = line.split(',');

        int numReadChannels; // 通道有效的信息
        if (separatedValues.length() >= int(_numOfChannels))
        {
            numReadChannels = _numOfChannels;
        }
        else // 如果有数据格式错误
        {
            numReadChannels = separatedValues.length();
            qWarning() << "Incoming data is missing data for some channels!";
        }

        // 解析读一行
        for (int ci = 0; ci < numReadChannels; ci++)
        {
            bool ok;
            double channelSample = separatedValues[ci].toDouble(&ok);
            if (ok)
            {
                addChannelData(ci, &channelSample, 1);
            }
            else
            {
                qWarning() << "Data parsing error for channel: " << ci;
            }
        }
        emit dataAdded();
    }
}

template<typename T> double DataFormatPanel::readSampleAs()
{
    T data;
    serialPort->read((char*) &data, sizeof(data));

    if (ui->rbLittleE->isChecked())
    {
        data = qFromLittleEndian(data);
    }
    else
    {
        data = qFromBigEndian(data);
    }

    return double(data);
}

void DataFormatPanel::addChannelData(unsigned int channel,
                                     double* data, unsigned size)
{
  //  (*_channelBuffers)[channel]->addSamples(data, size);
   // qDebug()<<data;
    if(isStartDetect){
        for(unsigned i = 0; i < size; i+=36)
        {
            ++clotingCount[channel];
            int sum = 0;
            int ave = 0;
            int xmax = 0;
            int xmin = 0;
            int ymax = 0;
            int ymin = 5000;
            unsigned int j;

            for(j = i; j< i+36; j++){

                sum += data[j];
            }
            ave = sum/36;
            QPointF s(double(j+18+count[channel])*0.005,ave);
            (*_channelBuffers)[channel]->append(s);
            (*_baseElasticchannelBuffers)[channel]->append(s);

            //存储凝血曲线
            while(clotingCount[channel] == 55)
            {
                qDebug()<<(*_channelBuffers)[channel]->size();

                //调节基线
                for(unsigned i = (*_channelBuffers)[channel]->size();
                    i>(*_channelBuffers)[channel]->size()-55;i--){
                    if((*_channelBuffers)[channel]->sample(i-1).ry()> ymax){
                        ymax = (*_channelBuffers)[channel]->sample(i-1).ry();
                        xmax = i-1;
                    }
                    if((*_channelBuffers)[channel]->sample(i-1).ry()< ymin){
                        ymin = (*_channelBuffers)[channel]->sample(i-1).ry();
                        xmin = i-1;
                    }
                }
                if(startBaseTested){
                    emit sendMaxMinValue(channel,ymax,ymin);
                }

                    emit sendElasticMaxMinValue(channel,ymax,ymin);
                if(!isStartDetectFlag[channel]){
                    emit sendBaseMaxValue(channel,ymax,ymin,(*_channelBuffers)[channel]->sample(xmax).x());
                    isStartDetectFlag[channel] = 1;
                }
                if(!isStopDetectFlag[channel]&&ymax-ymin<=5){
                    emit sendBaseMinValue(channel,ymax,ymin,(*_channelBuffers)[channel]->sample(xmax).x());
                    isStopDetectFlag[channel] = 1;
                }


                if(ymax>=((ymax+ymin)/2+20)){
                    if(ymax>=((ymax+ymin)/2+50)){
                        //测量K值
                        if(ymax<=((ymax+ymin)/2+500)){
                            KvaluePoint = (*_channelBuffers)[channel]->sample(xmax);
                        }
                        if(ymax>=((ymax+ymin)/2+500) && (ymax<=((ymax+ymin)/2+700))){

                             emit setKvalue(channel,KvaluePoint);
                        }

                        (*_maxclotingBuffers)[channel]->append((*_channelBuffers)[channel]->sample(xmax));
                        (*_minclotingBuffers)[channel]->append((*_channelBuffers)[channel]->sample(xmin));

                        //设置MA值
                        if((*_channelBuffers)[channel]->sample(xmin).ry() < MAvaluePoint.ry()){
                            MAvaluePoint = (*_channelBuffers)[channel]->sample(xmin);
                            double baseValue = (ymax+ymin)/2;
                            emit setMAvalue(channel,MAvaluePoint,baseValue);
                        }

                        //设置Angle角度
                        lineParaPoint = linefunc(RvaluePoint,(*_channelBuffers)[channel]->sample(xmax));
                        if(((*_maxclotingBuffers)[channel]->sample((*_maxclotingBuffers)[channel]->size()-2).y()
                                < getlineY(lineParaPoint,(*_maxclotingBuffers)[channel]->sample((*_maxclotingBuffers)[channel]->size()-2).x()))
                                &&(((*_maxclotingBuffers)[channel]->sample((*_maxclotingBuffers)[channel]->size()-3).y()
                                    < getlineY(lineParaPoint,(*_maxclotingBuffers)[channel]->sample((*_maxclotingBuffers)[channel]->size()-3).x())))){
                            AnglevaluePoint = (*_maxclotingBuffers)[channel]->sample((*_maxclotingBuffers)[channel]->size()-1);
                            emit setAnglevalue(channel,AnglevaluePoint,lineParaPoint);
                        }


                    }else{
                        (*_maxclotingBuffers)[channel]->append((*_channelBuffers)[channel]->sample(xmax));
                        (*_minclotingBuffers)[channel]->append((*_channelBuffers)[channel]->sample(xmin));
                        RvaluePoint = (*_channelBuffers)[channel]->sample(xmax);
                        emit setRvalue(channel,preRvaluePoint,RvaluePoint);
                    }

                //测量R值
                }else{
                    (*_maxclotingBuffers)[channel]->append(QPointF((*_channelBuffers)[channel]
                                                                   ->sample(xmax).rx(),(ymax+ymin)/2));
                    (*_minclotingBuffers)[channel]->append(QPointF((*_channelBuffers)[channel]
                                                                   ->sample(xmax).rx(),(ymax+ymin)/2));
                    preRvaluePoint = (*_channelBuffers)[channel]->sample(xmax);
                    emit setRvalue(channel,preRvaluePoint,preRvaluePoint);
                }
                 clotingCount[channel] = 0;
            }
        }

        count[channel] += size;
        sampleCount += size;
    }


}


QPointF DataFormatPanel::linefunc(QPointF point1, QPointF point2)
{
    QPointF linepara;
    linepara.setX((point1.ry()-point2.ry())/(point1.rx()-point2.rx()));
    linepara.setY(point1.ry()-(linepara.rx()*point1.rx()));
    return linepara;
}

double DataFormatPanel::getlineY(QPointF lineSlope, double xValue)
{
    double  yValue;
    yValue = (lineSlope.rx()*xValue)+lineSlope.ry();
    return yValue;
}

void DataFormatPanel::startDetect(bool enabled)
{
    isStartDetect = enabled;
}
