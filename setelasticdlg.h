#ifndef SETELASTICDLG_H
#define SETELASTICDLG_H

#include <QDialog>

namespace Ui {
class SetElasticDlg;
}

class SetElasticDlg : public QDialog
{
    Q_OBJECT

public:
    explicit SetElasticDlg(QWidget *parent = 0);
    ~SetElasticDlg();


    double elasticValue;
    double errorRangeValue;

private slots:
    void on_pbOK_clicked();

    void on_pbCancel_clicked();

private:
    Ui::SetElasticDlg *ui;
};

#endif // SETELASTICDLG_H
