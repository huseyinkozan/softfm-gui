#include "SettingsDialog.h"
#include "ui_SettingsDialog.h"

#include <QFileDialog>
#include <QSettings>

SettingsDialog::SettingsDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::SettingsDialog)
{
    ui->setupUi(this);
}

SettingsDialog::~SettingsDialog()
{
    delete ui;
}

QString SettingsDialog::softfm() const
{
    return QSettings().value("Settings/softfm",
                             ui->softfmLineEdit->text()).toString();
}

void SettingsDialog::on_cancelButton_clicked()
{
    reject();
}


void SettingsDialog::on_saveButton_clicked()
{
    QSettings s;
    s.beginGroup("Settings");
    s.setValue("softfm", ui->softfmLineEdit->text());
    s.endGroup();

    accept();
}


void SettingsDialog::on_softfmButton_clicked()
{
    QString fn = QFileDialog::getOpenFileName(
                this,
                tr("Select SoftFM"),
                qApp->applicationDirPath());
    if (fn.isEmpty())
        return;
    ui->softfmLineEdit->setText(fn);
}

