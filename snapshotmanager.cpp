
#include <QTime>
#include <QMenuBar>
#include <QKeySequence>
#include <QFileDialog>
#include <QFile>
#include <QVector>
#include <QPointF>

#include "snapshotmanager.h"

SnapshotManager::SnapshotManager(QMainWindow* mainWindow,
                                 QList<SignalData*>* channelBuffers) :
    _menu(tr("照相(&Snapshot)")),
    _takeSnapshotAction(tr("抓拍数据"), this),
    loadSnapshotAction(tr("导入数据"), this),
    clearAction(tr("清除数据"), this)
{
    _mainWindow = mainWindow;
    _channelBuffers = channelBuffers;

    _takeSnapshotAction.setToolTip(tr("拍摄目前的绘图区域的曲线"));
    _takeSnapshotAction.setIcon(QPixmap(":/new/toolbar/toolbarImage/Photo_678px_1183654_easyicon.net.png"));
    _takeSnapshotAction.setShortcut(QKeySequence("F5"));
    loadSnapshotAction.setToolTip(tr("Load snapshots from CSV files"));
    clearAction.setToolTip("Delete all snapshots");
    connect(&_takeSnapshotAction, SIGNAL(triggered(bool)),
            this, SLOT(takeSnapshot()));
    connect(&clearAction, SIGNAL(triggered(bool)),
            this, SLOT(clearSnapshots()));
    connect(&loadSnapshotAction, SIGNAL(triggered(bool)),
            this, SLOT(loadSnapshots()));

    updateMenu();
}

SnapshotManager::~SnapshotManager()
{
    for (auto snapshot : snapshots)
    {
        delete snapshot;
    }
}

void SnapshotManager::takeSnapshot()
{
    QString name = QTime::currentTime().toString("'Snapshot ['HH:mm:ss']'");
    auto snapshot = new Snapshot(_mainWindow, name);

    unsigned numOfChannels = _channelBuffers->size();



    for (unsigned ci = 0; ci < numOfChannels; ci++)
    {
        unsigned numOfSamples = _channelBuffers->at(ci)->size();
        snapshot->data.append(QVector<QPointF>(numOfSamples));
        for (unsigned i = 0; i < numOfSamples; i++)
        {
            snapshot->data[ci][i] = _channelBuffers->at(ci)->sample(i);
        }
    }

    addSnapshot(snapshot);
}

void SnapshotManager::addSnapshot(Snapshot* snapshot, bool update_menu)
{
    snapshots.append(snapshot);
    QObject::connect(snapshot, &Snapshot::deleteRequested,
                     this, &SnapshotManager::deleteSnapshot);
    if (update_menu) updateMenu();
}

void SnapshotManager::updateMenu()
{
    _menu.clear();
    _menu.addAction(&_takeSnapshotAction);
    _menu.addAction(&loadSnapshotAction);
    if (snapshots.size())
    {
        _menu.addSeparator();
        for (auto ss : snapshots)
        {
            _menu.addAction(ss->showAction());
        }
        _menu.addSeparator();
        _menu.addAction(&clearAction);
    }
}

void SnapshotManager::clearSnapshots()
{
    for (auto snapshot : snapshots)
    {
        delete snapshot;
    }
    snapshots.clear();
    updateMenu();
}

void SnapshotManager::deleteSnapshot(Snapshot* snapshot)
{
    snapshots.removeOne(snapshot);
    snapshot->deleteLater(); // regular delete causes a crash when triggered from menu
    updateMenu();
}

void SnapshotManager::loadSnapshots()
{
    auto files = QFileDialog::getOpenFileNames(_mainWindow, tr("Load CSV File"));

    for (auto f : files)
    {
        if (!f.isNull()) loadSnapshotFromFile(f);
    }

    updateMenu();
}

void SnapshotManager::loadSnapshotFromFile(QString fileName)
{
    QFile file(fileName);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        qCritical() << "Couldn't open file: " << fileName;
        qCritical() << file.errorString();
        return;
    }

    // 通过读取第一行来确定通道数
    auto headLine = QString(file.readLine());
    unsigned numOfChannels = headLine.split(',').size();

    // 读取数据
    QVector<QVector<QPointF>> data(numOfChannels);
    unsigned lineNum = 1;
    while (!file.atEnd())
    {
        // 计算行
        auto line = QString(file.readLine());
        auto split = line.split(',');

        if (split.size() != (int) numOfChannels)
        {
            qCritical() << "Parsing error at line " << lineNum
                        << ": number of columns is not consistent.";
            return;
        }

        for (unsigned ci = 0; ci < numOfChannels; ci++)
        {
            // 计算列
            bool ok;
            double y = split[ci].toDouble(&ok);
            if (!ok)
            {
                qCritical() << "Parsing error at line " << lineNum
                            << ", column " << ci
                            << ": can't convert \"" << split[ci]
                            << "\" to double.";
                return;
            }
            data[ci].append(QPointF(lineNum-1, y));
        }

        lineNum++;
    }

    auto snapshot = new Snapshot(_mainWindow, QFileInfo(fileName).baseName());
    snapshot->data = data;

    addSnapshot(snapshot, false);
}

QMenu* SnapshotManager::menu()
{
    return &_menu;
}

QAction* SnapshotManager::takeSnapshotAction()
{
    return &_takeSnapshotAction;
}
