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

QStringList SettingsDialog::commandArgs(double freq, bool forPreview) const
{
    const quint64 freqInt = freq * 1000000;
    const bool isAdv = forPreview ? ui->advancedModeCheckBox->isChecked() : isAdvancedMode();
    const QString dt = forPreview ? ui->deviceTypeComboBox->currentText() : deviceType();
    const bool isSte = isStereo();

    QStringList args;
    if ( ! isAdv)
        args << "-q";
    if ( ! isSte)
        args << "-M";
    args << "-t" << dt;
    args << "-c" << QString("freq=%1").arg(freqInt);

    return args;
}

bool SettingsDialog::isStereo() const
{
    return QSettings().value("Settings/isStereo", true).toBool();
}

void SettingsDialog::setIsStereo(bool isStereo)
{
    QSettings().setValue("Settings/isStereo", isStereo);
}

void SettingsDialog::showEvent(QShowEvent *event)
{
    ui->softfmLineEdit->setText(softfm());
    ui->deviceTypeComboBox->setCurrentText(deviceType());
    ui->advancedModeCheckBox->setChecked(isAdvancedMode());
    ui->darkModeCheckBox->setChecked(isDarkMode());
    updateCommandPreview();
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

void SettingsDialog::updateCommandPreview()
{
    QString text = QString("%1 %2").arg(softfm(), commandArgs(88.8, true).join(' '));
    ui->commandPreviewTextEdit->setText(text);
}

void SettingsDialog::on_advancedModeCheckBox_clicked()
{
    updateCommandPreview();
}


void SettingsDialog::on_deviceTypeComboBox_currentIndexChanged(int)
{
    updateCommandPreview();
}

