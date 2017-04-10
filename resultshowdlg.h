#ifndef RESULTSHOWDLG_H
#define RESULTSHOWDLG_H

#include <QDialog>

namespace Ui {
class ResultShowDlg;
}

class ResultShowDlg : public QDialog
{
    Q_OBJECT

public:
    explicit ResultShowDlg(QWidget *parent = 0);
    ~ResultShowDlg();

private:
    Ui::ResultShowDlg *ui;
};

#endif // RESULTSHOWDLG_H
