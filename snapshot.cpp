
#include <stddef.h>

#include "snapshot.h"
#include "snapshotview.h"

Snapshot::Snapshot(QMainWindow* parent, QString name) :
    QObject(parent),
    _showAction(name, this),
    _deleteAction(tr("删除"), this)
{
    _name = name;

    view = NULL;
    mainWindow = parent;
    connect(&_showAction, &QAction::triggered, this, &Snapshot::show);

    _deleteAction.setToolTip(QString("Delete ") + _name);
    connect(&_deleteAction, &QAction::triggered, this, &Snapshot::onDeleteTriggered);
}

Snapshot::~Snapshot()
{
    if (view != NULL)
    {
        delete view;
    }
}

QAction* Snapshot::showAction()
{
    return &_showAction;
}

QAction* Snapshot::deleteAction()
{
    return &_deleteAction;
}

void Snapshot::show()
{
    if (view == NULL)
    {
        view = new SnapshotView(mainWindow, this);
        connect(view, &SnapshotView::closed, this, &Snapshot::viewClosed);
    }
    view->show();
    view->activateWindow();
    view->raise();
}

void Snapshot::viewClosed()
{
    view->deleteLater();
    view = NULL;
}

void Snapshot::onDeleteTriggered()
{
    emit deleteRequested(this);
}

QString Snapshot::name()
{
    return _name;
}

void Snapshot::setName(QString name)
{
    _name = name;
    _showAction.setText(_name);
    emit nameChanged(this);
}
