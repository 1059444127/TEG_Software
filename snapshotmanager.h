
#ifndef SNAPSHOTMANAGER_H
#define SNAPSHOTMANAGER_H

#include <QObject>
#include <QAction>
#include <QMenu>

//#include "framebuffer.h"
#include "signaldata.h"
#include "snapshot.h"

class SnapshotManager : public QObject
{
    Q_OBJECT

public:
    SnapshotManager(QMainWindow* mainWindow, QList<SignalData*>* channelBuffers);
    ~SnapshotManager();

    QMenu* menu();
    QAction* takeSnapshotAction();

private:
    QMainWindow* _mainWindow;
    QList<SignalData*>* _channelBuffers;

    QList<Snapshot*> snapshots;

    QMenu _menu;
    QAction _takeSnapshotAction;
    QAction loadSnapshotAction;
    QAction clearAction;

    void addSnapshot(Snapshot* snapshot, bool update_menu=true);
    void updateMenu();

private slots:
    void takeSnapshot();
    void clearSnapshots();
    void deleteSnapshot(Snapshot* snapshot);
    void loadSnapshots();
    void loadSnapshotFromFile(QString fileName);
};

#endif /* SNAPSHOTMANAGER_H */
