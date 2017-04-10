
#ifndef COMMANDPANEL_H
#define COMMANDPANEL_H

#include <QWidget>
#include <QSerialPort>
#include <QByteArray>

#include "commandwidget.h"

namespace Ui {
class CommandPanel;
}

class CommandPanel : public QWidget
{
    Q_OBJECT

public:
    explicit CommandPanel(QSerialPort* port, QWidget *parent = 0);
    ~CommandPanel();

private:
    Ui::CommandPanel *ui;
    QSerialPort* serialPort;

private slots:
    void newCommand();
    void sendCommand(QByteArray command);
};

#endif // COMMANDPANEL_H
