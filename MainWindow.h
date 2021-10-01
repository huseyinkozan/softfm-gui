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

private:
    void handleAdvancedModeChange();

private:
    Ui::MainWindow *ui;
    bool m_isAdvancedMode = false;
    QLabel * m_versionLabel = nullptr;
    QProcess * m_process = nullptr;
    SettingsDialog * m_settingsDialog = nullptr;
};
#endif // MAINWINDOW_H
