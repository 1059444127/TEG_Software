#ifndef PRESETDLG_H
#define PRESETDLG_H

#include <QDialog>

namespace Ui {
class PresetDlg;
}

class PresetDlg : public QDialog
{
    Q_OBJECT

public:
    explicit PresetDlg(QWidget *parent = 0);
    ~PresetDlg();

private:
    Ui::PresetDlg *ui;
};

#endif // PRESETDLG_H
