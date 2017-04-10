
#include <QToolButton>
#include <QAction>
#include <QKeySequence>
#include <QHelpEvent>
#include <QToolTip>

#include "tooltipfilter.h"

bool ToolTipFilter::eventFilter(QObject *obj, QEvent *ev)
{
    //如果触发事件是或者是继承QToolButton
    if (ev->type() == QEvent::ToolTip && obj->inherits("QToolButton"))
    {
        // 设置工具栏的提示信息
        QToolButton* toolButton = (QToolButton*) obj;
        QAction* action = toolButton->defaultAction();

       if (action->toolTip().isNull()) return false;

       QString toolTip = action->toolTip();

      //  if (toolTip.isEmpty()) return false;

        QKeySequence keys = action->shortcut();
        if (!keys.isEmpty())
        {
            toolTip += QString(" <b>[") + keys.toString() + "]</b>";
        }

        // 显示工具栏按钮的提示信息
        QHelpEvent *helpEvent = static_cast<QHelpEvent *>(ev);
        QToolTip::showText(helpEvent->globalPos(), toolTip);
        return true;
    }
    else
    {
        return QObject::eventFilter(obj, ev);
    }
}
