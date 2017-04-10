
#ifndef COMMANDEDIT_H
#define COMMANDEDIT_H

#include <QLineEdit>
#include <QValidator>
#include <QString>

class CommandEdit : public QLineEdit
{
    Q_OBJECT

public:
    explicit CommandEdit(QWidget *parent = 0);
    ~CommandEdit();
    void setMode(bool ascii); // true = ascii, false = hex
    QString unEscapedText(); // return unescaped text(), used in ascii_mode only

private:
    bool ascii_mode;
    QValidator* hexValidator;
    QValidator* asciiValidator;

protected:
    void keyPressEvent(QKeyEvent * event) Q_DECL_OVERRIDE;
};

#endif // COMMANDEDIT_H
