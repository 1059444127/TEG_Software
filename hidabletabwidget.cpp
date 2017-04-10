
#include "hidabletabwidget.h"
#include <QTabBar>
#include <QToolButton>
#include <QSizePolicy>
#include <QTimer>

#define DOUBLE_CLICK_DELAY (200) // ms

HidableTabWidget::HidableTabWidget(QWidget *parent) :
    QTabWidget(parent),
    hideAction("▾", this)
{
    hideAction.setCheckable(true);
    hideAction.setToolTip("Hide Panels");
    QToolButton* hideButton = new QToolButton();
    hideButton->setDefaultAction(&hideAction);
    hideButton->setAutoRaise(true);
    this->setCornerWidget(hideButton);

    connect(&hideAction, SIGNAL(toggled(bool)), this, SLOT(onHideAction(bool)));
    connectSignals();
}

void HidableTabWidget::onHideAction(bool checked)
{
    if (checked) // hide
    {
        this->setMaximumHeight(this->tabBar()->height());
        disconnect(this, SIGNAL(tabBarDoubleClicked(int)), this, SLOT(onTabBarDoubleClicked()));
        QTimer::singleShot(DOUBLE_CLICK_DELAY, this, SLOT(connectSignals()));
    }
    else // show
    {
        this->setMaximumHeight(100000); // just a very big number
        disconnect(this, SIGNAL(tabBarClicked(int)), this, SLOT(onTabBarClicked()));
        QTimer::singleShot(DOUBLE_CLICK_DELAY, this, SLOT(connectSignals()));
    }
}

void HidableTabWidget::onTabBarClicked()
{
    hideAction.setChecked(false);
}

void HidableTabWidget::onTabBarDoubleClicked()
{
    hideAction.setChecked(true);
}

void HidableTabWidget::connectSignals()
{
    if (hideAction.isChecked()) // hidden
    {
        connect(this, SIGNAL(tabBarClicked(int)), this, SLOT(onTabBarClicked()));
    }
    else // shown
    {
        connect(this, SIGNAL(tabBarDoubleClicked(int)), this, SLOT(onTabBarDoubleClicked()));
    }
}
