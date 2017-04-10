
#ifndef COMMANDWIDGET_H
#define COMMANDWIDGET_H

#include <QWidget>
#include <QByteArray>

namespace Ui {
class CommandWidget;
}

class CommandWidget : public QWidget
{
    Q_OBJECT

public:
    explicit CommandWidget(QWidget *parent = 0);
    ~CommandWidget();

signals:
    void deleteRequested(CommandWidget* thisWidget); // emitted when delete button is clicked

    // emitted when send button is clicked
    //
    // in case of hex mode, command text should be a hexadecimal
    // string containing hexadecimal characters only (not even spaces)
    void sendCommand(QByteArray command);

private:
    Ui::CommandWidget *ui;

    bool isASCIIMode(); // true: ascii mode, false hex mode

private slots:
    void onDeleteClicked();
    void onSendClicked();
    void onASCIIToggled(bool checked);
};

#endif // COMMANDWIDGET_H
