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

QString SettingsDialog::customArgs() const
{
    return QSettings().value("Settings/customArgs", QString()).toString();
}

QString SettingsDialog::customConf() const
{
    return QSettings().value("Settings/customConf", QString()).toString();
}

QStringList SettingsDialog::commandArgs(int freqAsKhz, bool isScanning,
                                        bool isMuted, bool forPreview) const
{
    const quint64 freqHz = freqAsKhz * 1000;
    const QString dt = forPreview ? ui->deviceTypeComboBox->currentText() : deviceType();
    const QString ca = forPreview ? ui->customArgsLineEdit->text()        : customArgs();
    const QString cc = forPreview ? ui->customConfLineEdit->text()        : customConf();
    const bool    im = isMono();

    QStringList args;
    if (im)
        args << "-M";
    args << "-t" << dt;

    if (isScanning && isMuted)
        args << "-Pnull";

    QString conf = QString("freq=%1").arg(freqHz);
    if ( ! cc.trimmed().isEmpty())
        conf.append(',').append(cc);
    args << "-c" << conf;


    if ( ! ca.isEmpty()) {
        const QStringList customArgs = ca.trimmed().split(' ',
#if (QT_VERSION >= QT_VERSION_CHECK(5,14,0))
                                                          Qt::SkipEmptyParts
#else
                                                          QString::SkipEmptyParts
#endif
                                                          );
        for (int i = 0; i < customArgs.count(); ++i) {
            const QString & customArg = customArgs.at(i);
            if (isScanning) {
                if (customArg.toLower() != "-q")
                    args << customArg;
            }
            else {
                args << customArg;
            }
        }
    }

    return args;
}

bool SettingsDialog::isMono() const
{
    return QSettings().value("Settings/isMono", false).toBool();
}

void SettingsDialog::setIsMono(bool mono)
{
    QSettings().setValue("Settings/isMono", mono);
}

void SettingsDialog::showEvent(QShowEvent *event)
{
    ui->softfmLineEdit->setText(softfm());
    ui->deviceTypeComboBox->setCurrentText(deviceType());
    ui->advancedModeCheckBox->setChecked(isAdvancedMode());
    ui->darkModeCheckBox->setChecked(isDarkMode());
    ui->customArgsLineEdit->setText(customArgs());
    ui->customConfLineEdit->setText(customConf());
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
    s.setValue("customArgs", ui->customArgsLineEdit->text());
    s.setValue("customConf", ui->customConfLineEdit->text());
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
    QString text = QString("%1 %2")
            .arg(softfm(),commandArgs(88800, false, false, true).join(' '));
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


void SettingsDialog::on_customArgsLineEdit_textChanged(const QString &)
{
    updateCommandPreview();
}


void SettingsDialog::on_customConfLineEdit_textChanged(const QString &)
{
    updateCommandPreview();
}

