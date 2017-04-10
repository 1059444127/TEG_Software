#include "maintenancedlg.h"
#include "ui_maintenancedlg.h"
#include "setelasticdlg.h"

#include <QModelIndex>

MaintenanceDlg::MaintenanceDlg(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::MaintenanceDlg),
    isSetElasticed(false)
{
    ui->setupUi(this);
    ui->tableWidget->horizontalHeader()->resizeSection(0,50); //设置表头第一列的宽度为150
    ui->tableWidget->horizontalHeader()->setStyleSheet("QHeaderView::section{background:skyblue;}"); //设置表头背景色

    for(int i = 0 ; i < 9; i++){

        QString str = QString::number(i+1,'f',0);
        ui->tableWidget->setItem(i,0,new QTableWidgetItem(str));
    }
}

MaintenanceDlg::~MaintenanceDlg()
{
    delete ui;
}

void MaintenanceDlg::on_pbeTest_clicked(bool checked)
{
    emit starteTest(checked);
}

void MaintenanceDlg::onSetBaseValue(unsigned channel,double maxValue, double minValue)
{
    QString minStr = QString::number(minValue,'f',0);
    ui->tableWidget->setItem(channel,1,new QTableWidgetItem(minStr));

    QString maxStr = QString::number(maxValue,'f',0);
    ui->tableWidget->setItem(channel,2,new QTableWidgetItem(maxStr));


    QString diffStr = QString::number(maxValue-minValue,'f',0);
    ui->tableWidget->setItem(channel,3,new QTableWidgetItem(diffStr));

    QString mmStr = QString::number((maxValue-minValue)*0.0235,'f',1);
    ui->tableWidget->setItem(channel,4,new QTableWidgetItem(mmStr));

    if(isSetElasticed){
        if(((maxValue-minValue)*0.02<=(elasticValue+errorRangeValue))
                &&((maxValue-minValue)*0.02>=(elasticValue-errorRangeValue))){
             ui->tableWidget->setItem(channel,5,new QTableWidgetItem(tr("弹力值调节完成，可进行样品检验")));
        }else{
            ui->tableWidget->setItem(channel,5,new QTableWidgetItem(tr("弹力值未符合要求，请继续调整")));
        }
    }else{
        if((maxValue<=2200)&&(minValue>=1900)){
            ui->tableWidget->setItem(channel,5,new QTableWidgetItem(tr("基线已经调整至中心位置")));
        }else{
            ui->tableWidget->setItem(channel,5,new QTableWidgetItem(tr("基线没有调整至中心位置，请继续调整")));
        }
    }



}

void MaintenanceDlg::on_pbSetElastic_clicked()
{
    SetElasticDlg setelastic;
    if(setelastic.exec() == QDialog::Accepted){
          elasticValue = setelastic.elasticValue;
          errorRangeValue = setelastic.errorRangeValue;
          isSetElasticed = true;

    }

}
