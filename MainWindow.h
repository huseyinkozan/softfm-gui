#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QProcess>


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

    void settingsDialogFinished(int result);
    void on_actionSettings_triggered();

    void radioOff();
    void radioOn();
    void on_onButton_clicked(bool checked);
    void on_freqDoubleSpinBox_valueChanged(double);

    void on_clearButton_clicked();

private:
    void applyDarkMode();
    bool isRadioOn() const;
    void updateAdvancedModeFields(const QString & txt);
    void captureEvents(const QString & txt);

private:
    Ui::MainWindow *ui;
    QLabel * m_versionLabel = nullptr;
    QProcess * m_process = nullptr;
    SettingsDialog * m_settingsDialog = nullptr;
    bool m_isOnRequested = false;
    double m_freq = 0.0;
};
#endif // MAINWINDOW_H
