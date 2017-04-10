
#include <QApplication>
#include <QtGlobal>

#include "mainwindow.h"
#include "tooltipfilter.h"



MainWindow* pMainWindow;

void messageHandler(QtMsgType type, const QMessageLogContext &context,
                    const QString &msg)
{
    // TODO: don't call MainWindow::messageHandler if window is destroyed
    pMainWindow->messageHandler(type, context, msg);
}

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    MainWindow window;
    pMainWindow = &window;

    qInstallMessageHandler(messageHandler);

    //ToolTipFilter ttf;
    //app.installEventFilter(&ttf);
    window.show();
    return app.exec();
}
