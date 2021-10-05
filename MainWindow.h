#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QProcess>
#include <QSystemTrayIcon>
#include "main.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
class QLabel;
QT_END_NAMESPACE
class SettingsDialog;


class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    bool isAdvancedMode() const;
    void setIsAdvancedMode(bool newIsAdvancedMode);

protected:
    void closeEvent(QCloseEvent *event);
    void writeSettings();
    void readSettings();

private slots:
    void processFinished(int exitCode, QProcess::ExitStatus exitStatus);
    void processStateChanged(QProcess::ProcessState state);
    void processReadyReadStandardError();
    void processReadyReadStandardOutput();
    void processErrorOccurred(QProcess::ProcessError error);

    void on_actionQuit_triggered();
    void settingsDialogFinished(int result);
    void on_actionSettings_triggered();

    void radioOff();
    void radioOn();
    void changeFreq();
    void on_onButton_clicked(bool checked);
    void on_freqDoubleSpinBox_valueChanged(double);
    void on_freqDownButton_clicked();
    void on_freqUpButton_clicked();
    void on_clearButton_clicked();

    void fillTable();
    void tableActionRemove(double freq);
    void on_addButton_clicked();
    void selectTableRow(double freq);
    void on_tableWidget_itemSelectionChanged();

    void trayIconActivated(QSystemTrayIcon::ActivationReason reason);

private:
    void applyDarkMode();
    bool isRadioOn() const;
    void updateAdvancedModeFields(const QString & txt);
    void captureEvents(const QString & txt);
    FreqDescMap tableData() const;

private:
    Ui::MainWindow *ui;
    QLabel * m_versionLabel = nullptr;
    QProcess * m_process = nullptr;
    SettingsDialog * m_settingsDialog = nullptr;
    bool m_isOnRequested = false;
    double m_freq = 0.0;
    QSystemTrayIcon * m_trayIcon = nullptr;
    QTimer * m_changeFreqTimer = nullptr;
};
#endif // MAINWINDOW_H
