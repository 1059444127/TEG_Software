#include "presetdlg.h"
#include "ui_presetdlg.h"

PresetDlg::PresetDlg(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::PresetDlg)
{
    ui->setupUi(this);
    Qt::WindowFlags flags=Qt::Dialog;
    flags |=Qt::WindowMinimizeButtonHint;
    flags |=Qt::WindowMaximizeButtonHint;
    flags |=Qt::WindowCloseButtonHint;
    setWindowFlags(flags);
}

PresetDlg::~PresetDlg()
{
    delete ui;
}
