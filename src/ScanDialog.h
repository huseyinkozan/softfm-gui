#ifndef SCANDIALOG_H
#define SCANDIALOG_H

#include <QDialog>
#include "ChannelRecord.h"
#include "MainWindow.h"


namespace Ui {
class ScanDialog;
}


class ScanDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ScanDialog(QWidget *parent = nullptr);
    ~ScanDialog();

    const ChannelRecordMap &crMap() const;
    void setCrMap(const ChannelRecordMap &newCrMap);

    void process(const MainWindow::AdvancedFields & af);

public slots:
    void done(int result);

signals:
    void changeFreq(int freqAsKhz);

protected:
    void showEvent(QShowEvent *event);

private slots:
    void timerTimeout();

    void on_scanButton_clicked();
    void on_okButton_clicked();
    void on_cancelButton_clicked();

    void on_tableWidget_itemSelectionChanged();

private:
    Ui::ScanDialog *ui;
    ChannelRecordMap m_crMap;
    QTimer * m_timer = nullptr;
};

#endif // SCANDIALOG_H
