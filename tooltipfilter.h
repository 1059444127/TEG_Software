
#ifndef TOOLTIPFILTER_H
#define TOOLTIPFILTER_H

#include <QObject>
#include <QEvent>

class ToolTipFilter : public QObject
{
protected:
    bool eventFilter(QObject *obj, QEvent *ev);
};

#endif /* TOOLTIPFILTER_H */
