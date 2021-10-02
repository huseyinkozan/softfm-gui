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
    return QSettings().value("Settings/softfm", ui->softfmLineEdit->text()).toString();
}

QString SettingsDialog::deviceType() const
{
    return QSettings().value("Settings/deviceType", "rtlsdr").toString();
}

bool SettingsDialog::isAdvancedMode() const
{
    return QSettings().value("Settings/isAdvancedMode", false).toBool();
}

bool SettingsDialog::isDarkMode() const
{
    return QSettings().value("Settings/isDarkMode", false).toBool();
}

void SettingsDialog::showEvent(QShowEvent *event)
{
    ui->softfmLineEdit->setText(softfm());
    ui->deviceTypeComboBox->setCurrentText(deviceType());
    ui->advancedModeCheckBox->setChecked(isAdvancedMode());
    ui->darkModeCheckBox->setChecked(isDarkMode());
    QDialog::showEvent(event);
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
    s.setValue("deviceType", ui->deviceTypeComboBox->currentText());
    s.setValue("isAdvancedMode", ui->advancedModeCheckBox->isChecked());
    s.setValue("isDarkMode", ui->darkModeCheckBox->isChecked());
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

