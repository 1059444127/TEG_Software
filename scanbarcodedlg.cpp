#include "scanbarcodedlg.h"
#include "ui_scanbarcodedlg.h"

ScanBarCodeDlg::ScanBarCodeDlg(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ScanBarCodeDlg)
{
    ui->setupUi(this);
}

ScanBarCodeDlg::~ScanBarCodeDlg()
{
    delete ui;
}
