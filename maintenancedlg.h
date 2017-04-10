#ifndef MAINTENANCEDLG_H
#define MAINTENANCEDLG_H

#include <QDialog>
#include <plot.h>


namespace Ui {
class MaintenanceDlg;
}

class MaintenanceDlg : public QDialog
{
    Q_OBJECT

public:
    explicit MaintenanceDlg(QWidget *parent = 0);
    ~MaintenanceDlg();
    double elasticValue;
    double errorRangeValue;
signals:
    starteTest(bool);

public slots:
    void onSetBaseValue(unsigned channel, double maxValue,double minValue);

private slots:
    void on_pbeTest_clicked(bool checked);
    void on_pbSetElastic_clicked();

private:
    Ui::MaintenanceDlg *ui;
    bool isSetElasticed;


};

#endif // MAINTENANCEDLG_H
