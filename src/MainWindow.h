#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QProcess>
#include <QSystemTrayIcon>
#include "main.h"
#include "ChannelRecord.h"

QT_BEGIN_NAMESPACE

class QTableWidgetItem;
namespace Ui { class MainWindow; }
class QLabel;
QT_END_NAMESPACE
class ScanDialog;
class SettingsDialog;


class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    struct AdvancedFields {
        bool    isValid     = false;
        quint64 blk         = 0;
        qreal   freqAsMhz   = 0.0;
        qreal   ppm         = 0.0;
        qreal   if_         = 0.0;
        qreal   bb          = 0.0;
        qreal   audio       = 0.0;
        qreal   buf         = 0.0;
        AdvancedFields() {}
        QString toString() {
            return QString("blk= %1 freq= %2MHz ppm= %3 IF= %4dB BB= %5dB audio= %6dB buf= %7s isValid=%8")
                    .arg(blk)
                    .arg(freqAsMhz, 0, 'f', 6)
                    .arg(ppm, 0, 'f', 2)
                    .arg(if_, 0, 'f', 1)
                    .arg(bb, 0, 'f', 1)
                    .arg(audio, 0, 'f', 1)
                    .arg(buf, 0, 'f', 1)
                    .arg(isValid?"Y":"N")
                    ;
        }
    };

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

    void on_actionExit_triggered();
    void settingsDialogFinished(int result);
    void on_actionSettings_triggered();
    void on_actionPreview_triggered(bool checked);

    void on_actionScan_triggered();
    void scanDialogChangeFreq(int freqAsKhz);
    void scanDialogFinished(int result);

    void stopProcess(int waitForMsec = 300);
    void radioOff();
    void radioOn();
    void changeFreq();
    void on_actionOn_triggered(bool checked);
    void on_freqDoubleSpinBox_valueChanged(double);
    void on_freqDownButton_clicked();
    void on_freqUpButton_clicked();
    void on_clearButton_clicked();

    void fillTable();
    void tableActionRemove(int freqAsKhz);
    void on_addButton_clicked();
    void selectTableRow(int freqAsKhz);
    void on_tableWidget_itemSelectionChanged();
    void on_tableWidget_itemChanged(QTableWidgetItem *item);
    void previewTimerTimeout();
    void previewProgressTimerTimeout();

    void trayIconActivated(QSystemTrayIcon::ActivationReason reason);
    void trayIconActionTriggered(bool checked);
    void fillTrayIconMenu();
    void updateTrayIconActionChecks();

    void on_monoButton_toggled(bool checked);


private:
    void applyDarkMode();
    bool isRadioOn() const;
    AdvancedFields getAdvancedFields(const QString & txt) const;
    void updateAdvancedModeFields(const AdvancedFields & af);
    void captureEvents(const QString & txt);

    ChannelRecordMap getChannelRecordMap() const;
    void setChannelRecordMap(const ChannelRecordMap & crMap);

private:
    Ui::MainWindow *ui;
    QAction * m_logDockAction = nullptr;
    QLabel * m_stereoLabel = nullptr;
    QLabel * m_versionLabel = nullptr;
    QProcess * m_process = nullptr;
    ScanDialog * m_scanDialog = nullptr;
    SettingsDialog * m_settingsDialog = nullptr;
    bool m_isScanning = false;
    bool m_isOnRequested = false;
    int m_freqAsKhs = 0;
    QSystemTrayIcon * m_trayIcon = nullptr;
    QMenu * m_trayIconMenu = nullptr;
    QTimer * m_changeFreqTimer = nullptr;
    QTimer * m_previewTimer = nullptr;
    QTimer * m_previewProgressTimer = nullptr;
    qint64   m_previewChanged = -1;
    ChannelRecordMap m_crMap;
};
#endif // MAINWINDOW_H
