
#include <algorithm>
#include <QPen>

#include "plotsnapshotoverlay.h"

#define LINE_WIDTH (10)
#define ANIM_LENGTH (500) // milliseconds
#define UPDATE_PERIOD (20) // milliseconds

PlotSnapshotOverlay::PlotSnapshotOverlay(QWidget* widget, QColor color) :
    QwtWidgetOverlay(widget)
{
    _color = color;
    animTimer.setSingleShot(true);
    animTimer.setInterval(ANIM_LENGTH);
    updateTimer.setSingleShot(false);
    updateTimer.setInterval(UPDATE_PERIOD);

    connect(&updateTimer, &QTimer::timeout, [this](){this->updateOverlay();});
    connect(&animTimer, &QTimer::timeout, &updateTimer, &QTimer::stop);
    connect(&animTimer, &QTimer::timeout, this, &PlotSnapshotOverlay::done);

    animTimer.start();
    updateTimer.start();
}

void PlotSnapshotOverlay::drawOverlay(QPainter* painter) const
{
        if (!animTimer.isActive()) return;
    QColor lineColor = _color;

    double fadingRatio = ((double) animTimer.remainingTime()) / ANIM_LENGTH;
    lineColor.setAlpha(std::min(255, (int) (255*fadingRatio)));

    QPen pen(lineColor);
    pen.setWidth(LINE_WIDTH);
    pen.setJoinStyle(Qt::MiterJoin);

    int width = painter->device()->width();
    int height = painter->device()->height();

    painter->save();
    painter->setPen(pen);
    painter->drawRect(LINE_WIDTH/2, LINE_WIDTH/2, width-LINE_WIDTH, height-LINE_WIDTH);
    painter->restore();
}
