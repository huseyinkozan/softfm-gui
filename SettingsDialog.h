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
    bool isAdvancedMode() const;
    bool isDarkMode() const;

protected:
    void showEvent(QShowEvent *event);

private slots:
    void on_cancelButton_clicked();
    void on_saveButton_clicked();

    void on_softfmButton_clicked();

private:
    Ui::SettingsDialog *ui;
};

#endif // SETTINGSDIALOG_H
