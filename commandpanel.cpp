
#include "commandpanel.h"
#include "ui_commandpanel.h"

#include <QByteArray>
#include <QtDebug>

CommandPanel::CommandPanel(QSerialPort* port, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::CommandPanel)
{
    serialPort = port;

    ui->setupUi(this);
    ui->scrollAreaWidgetContents->setLayout(new QVBoxLayout);

#ifdef Q_OS_WIN
    ui->pbNew->setIcon(QIcon(":/icons/list-add"));
#endif // Q_OS_WIN

    connect(ui->pbNew, &QPushButton::clicked, this, &CommandPanel::newCommand);

    newCommand(); // add an empty slot by default
}

CommandPanel::~CommandPanel()
{
    delete ui;
}

void CommandPanel::newCommand()
{
    auto command = new CommandWidget();
    ui->scrollAreaWidgetContents->layout()->addWidget(command);
    connect(command, &CommandWidget::sendCommand, this, &CommandPanel::sendCommand);
}

void CommandPanel::sendCommand(QByteArray command)
{
    if (!serialPort->isOpen())
    {
        qCritical() << "Port is not open!";
        return;
    }

    if (serialPort->write(command) < 0)
    {
        qCritical() << "Send command failed!";
    }
}
