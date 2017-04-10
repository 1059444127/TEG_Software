
#ifndef SNAPSHOT_H
#define SNAPSHOT_H

#include <QObject>
#include <QMainWindow>
#include <QAction>
#include <QVector>
#include <QString>

class SnapshotView;

class Snapshot : public QObject
{
    Q_OBJECT

public:
    Snapshot(QMainWindow* parent, QString name);
    ~Snapshot();

    QVector<QVector<QPointF>> data;
    QVector<QPointF> mag_data;
    QAction* showAction();
    QAction* deleteAction();

    QString name();
    void setName(QString name);

signals:
    void deleteRequested(Snapshot*);
    void nameChanged(Snapshot*);

private:
    QString _name;
    QAction _showAction;
    QAction _deleteAction;
    QMainWindow* mainWindow;
    SnapshotView* view;

private slots:
    void show();
    void viewClosed();

    void onDeleteTriggered();
};

#endif /* SNAPSHOT_H */
