#ifndef SETTINGSDIALOG_H
#define SETTINGSDIALOG_H

#include <QDialog>

namespace Ui {
class SettingsDialog;
}

class SettingsDialog : public QDialog
{
    Q_OBJECT

public:
    explicit SettingsDialog(QWidget *parent = nullptr);
    ~SettingsDialog();

    QString softfm() const;
    QString deviceType() const;
    bool isAdvancedMode() const;
    bool isDarkMode() const;
    QStringList commandArgs(double freq, bool forPreview = false) const;

    bool isStereo() const;
    void setIsStereo(bool isStereo);

protected:
    void showEvent(QShowEvent *event);

private slots:
    void on_cancelButton_clicked();
    void on_saveButton_clicked();
    void on_softfmButton_clicked();

    void on_advancedModeCheckBox_clicked();
    void on_deviceTypeComboBox_currentIndexChanged(int);

private:
    void updateCommandPreview();

private:
    Ui::SettingsDialog *ui;
};

#endif // SETTINGSDIALOG_H
