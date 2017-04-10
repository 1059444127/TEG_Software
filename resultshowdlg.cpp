#include "resultshowdlg.h"
#include "ui_resultshowdlg.h"

ResultShowDlg::ResultShowDlg(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ResultShowDlg)
{
    ui->setupUi(this);
}

ResultShowDlg::~ResultShowDlg()
{
    delete ui;
}
