#include "setelasticdlg.h"
#include "ui_setelasticdlg.h"
#include <QMessageBox>

SetElasticDlg::SetElasticDlg(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::SetElasticDlg)
{
    ui->setupUi(this);
    QRegExp ElasticRegExp("[0-9][0-9][.][0-9]");
    ui->leElasticValue->setValidator(new QRegExpValidator(ElasticRegExp,this));

    QRegExp ErrorRegExp("[0-9][.][0-9]");
    ui->leErrorRange->setValidator(new QRegExpValidator(ErrorRegExp,this));
}

SetElasticDlg::~SetElasticDlg()
{
    delete ui;
}

void SetElasticDlg::on_pbOK_clicked()
{
    if(ui->leElasticValue->displayText().isEmpty()||
            ui->leErrorRange->displayText().isEmpty()){
        QMessageBox::warning(this, tr("警告"),
                                       tr("请输入正确格式的弹力器值和误差范围"));
        return ;
    }

    elasticValue = ui->leElasticValue->displayText().toDouble();
    errorRangeValue = ui->leErrorRange->displayText().toDouble();
    accept();
}

void SetElasticDlg::on_pbCancel_clicked()
{
    close();
}
