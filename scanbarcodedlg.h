#ifndef SCANBARCODEDLG_H
#define SCANBARCODEDLG_H

#include <QDialog>

namespace Ui {
class ScanBarCodeDlg;
}

class ScanBarCodeDlg : public QDialog
{
    Q_OBJECT

public:
    explicit ScanBarCodeDlg(QWidget *parent = 0);
    ~ScanBarCodeDlg();

private:
    Ui::ScanBarCodeDlg *ui;
};

#endif // SCANBARCODEDLG_H
