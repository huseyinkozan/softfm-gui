#include "MainWindow.h"
#include "ui_MainWindow.h"

#include <QCloseEvent>
#include <QDateTime>
#include <QDebug>
#include <QInputDialog>
#include <QLabel>
#include <QSettings>
#include <QTextStream>
#include <QTimer>

#include "ScanDialog.h"
#include "SettingsDialog.h"
#include "TableActionWidget.h"
#include "CheckBoxDelegate.h"
#include "FreqItemDelegate.h"


enum Columns {
    Column_Freq = 0,
    Column_Desc,
    Column_Tray,
    Column_Act,
    Column__Max_
};

enum TableTypes {
    TableType_Freq = QTableWidgetItem::UserType +  1,
    TableType_Desc = QTableWidgetItem::UserType +  2,
    TableType_Tray = QTableWidgetItem::UserType +  3,
    TableType_Act  = QTableWidgetItem::UserType +  4,
};



MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    m_logDockAction = ui->logDockWidget->toggleViewAction();
    m_logDockAction->setObjectName("logDockAction");
    m_logDockAction->setIcon(QIcon(":images/log.svg"));
    m_logDockAction->setStatusTip(tr("Show Log"));
    ui->toolBar->insertAction(ui->actionExit, m_logDockAction);

    QWidget * exp1 = new QWidget(this);
    exp1->setObjectName("exp1");
    exp1->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Expanding);
    exp1->setStyleSheet("background-color:transparent");
    ui->toolBar->insertWidget(ui->actionExit, exp1);

    QList<QAction*> actionList = ui->toolBar->actions();
    for (int i = 0; i < actionList.count(); ++i) {
        QAction * a = actionList.at(i);
        QWidget * w = ui->toolBar->widgetForAction(a);
        if ( ! w) continue;
        QToolButton * tb = qobject_cast<QToolButton*>(w);
        if ( ! tb) continue;
        tb->setMinimumWidth(70);
        tb->setAutoRaise(true);
    }

    QTextDocument * logDoc = ui->logTextEdit->document();
    if (logDoc)
        logDoc->setMaximumBlockCount(1000);

    m_stereoLabel = new QLabel("");
    m_stereoLabel->setObjectName("m_stereoLabel");
    m_stereoLabel->setStatusTip(tr("Shows stereo info"));
    statusBar()->addPermanentWidget(m_stereoLabel);

    m_versionLabel = new QLabel(tr("%1").arg(qApp->applicationVersion()));
    m_versionLabel->setObjectName("m_versionLabel");
    m_versionLabel->setEnabled(false);
    m_versionLabel->setStatusTip(tr("Version"));
    statusBar()->addPermanentWidget(m_versionLabel);

    m_freqAsKhs = mhz_to_khz(ui->freqDoubleSpinBox->value());

    QStringList headerList = QStringList()
            << tr("Freq")
            << tr("Desc")
            << tr("Tray")
            << tr("Act")
               ;
    Q_ASSERT(headerList.count() == Column__Max_);
    ui->tableWidget->setColumnCount(headerList.count());
    ui->tableWidget->setHorizontalHeaderLabels(headerList);
    QHeaderView * hv = ui->tableWidget->horizontalHeader();
    Q_CHECK_PTR(hv);
    hv->setSectionResizeMode(Column_Freq, QHeaderView::Interactive);
    hv->setSectionResizeMode(Column_Desc, QHeaderView::Stretch);
    hv->setSectionResizeMode(Column_Tray, QHeaderView::Fixed);
    hv->setSectionResizeMode(Column_Act,  QHeaderView::Fixed);
    hv->resizeSection(Column_Tray, 40);
    hv->resizeSection(Column_Act, 40);
    ui->tableWidget->horizontalHeaderItem(Column_Freq)->setStatusTip(tr("Frequency"));
    ui->tableWidget->horizontalHeaderItem(Column_Desc)->setStatusTip(tr("Description"));
    ui->tableWidget->horizontalHeaderItem(Column_Tray)->setStatusTip(tr("Show at tray"));
    ui->tableWidget->horizontalHeaderItem(Column_Act )->setStatusTip(tr("Action"));

    FreqItemDelegate * freqColItemDlg = new FreqItemDelegate(this);
    ui->tableWidget->setItemDelegateForColumn(Column_Freq, freqColItemDlg);

    CheckBoxDelegate * trayColItemDlg = new CheckBoxDelegate(this);
    trayColItemDlg->setLabelText(tr("Show"));
    ui->tableWidget->setItemDelegateForColumn(Column_Tray, trayColItemDlg);

    m_process = new QProcess(this);
    connect(m_process, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished), this, &MainWindow::processFinished);
    connect(m_process, &QProcess::stateChanged,                                       this, &MainWindow::processStateChanged);
    connect(m_process, &QProcess::readyReadStandardError,                             this, &MainWindow::processReadyReadStandardError);
    connect(m_process, &QProcess::readyReadStandardOutput,                            this, &MainWindow::processReadyReadStandardOutput);
    connect(m_process, &QProcess::errorOccurred,                                      this, &MainWindow::processErrorOccurred);

    m_scanDialog = new ScanDialog(this);
    connect(m_scanDialog, &ScanDialog::changeFreq, this, &MainWindow::scanDialogChangeFreq);
    connect(m_scanDialog, &ScanDialog::finished,   this, &MainWindow::scanDialogFinished);

    m_settingsDialog = new SettingsDialog(this);
    m_settingsDialog->setObjectName("m_settingsDialog");
    connect(m_settingsDialog, &SettingsDialog::finished, this, &MainWindow::settingsDialogFinished);



    m_trayIcon = new QSystemTrayIcon(this);
    m_trayIcon->setObjectName("m_trayIcon");
    m_trayIcon->setIcon(QIcon(":/images/tray-icon.svg"));
    m_trayIcon->show();
    connect(m_trayIcon, &QSystemTrayIcon::activated, this, &MainWindow::trayIconActivated);

    m_trayIconMenu = new QMenu(this);
    m_trayIcon->setContextMenu(m_trayIconMenu);
    fillTrayIconMenu();

    m_changeFreqTimer = new QTimer(this);
    m_changeFreqTimer->setObjectName("m_changeFreqTimer");
    m_changeFreqTimer->setSingleShot(true);
    connect(m_changeFreqTimer, &QTimer::timeout, this, &MainWindow::changeFreq);

    m_previewTimer = new QTimer(this);
    m_previewTimer->setObjectName("m_previewTimer");
    m_previewTimer->setSingleShot(false);
    connect(m_previewTimer, &QTimer::timeout, this, &MainWindow::previewTimerTimeout);

    m_previewProgressTimer = new QTimer(this);
    m_previewProgressTimer->setObjectName("m_previewProgressTimer");
    m_previewProgressTimer->setSingleShot(false);
    connect(m_previewProgressTimer, &QTimer::timeout, this, &MainWindow::previewProgressTimerTimeout);

    readSettings();

    QTimer::singleShot(0, this, [this]{
        settingsDialogFinished(SettingsDialog::Accepted);
        fillTable();
    });
}

MainWindow::~MainWindow()
{
    m_isOnRequested = false;
    stopProcess(1500);

    delete ui;
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    writeSettings();
    event->accept();
}

void MainWindow::writeSettings()
{
    QSettings s;
    s.beginGroup("MainWindow");
    s.setValue("geometry", saveGeometry());
    s.setValue("state", saveState());
    s.endGroup();
}

void MainWindow::readSettings()
{
    QSettings s;
    s.beginGroup("MainWindow");
    restoreGeometry(s.value("geometry").toByteArray());
    restoreState(s.value("state").toByteArray());
    s.endGroup();
}

void MainWindow::processFinished(int exitCode, QProcess::ExitStatus exitStatus)
{
    m_stereoLabel->setText("");

    if (exitStatus != QProcess::NormalExit) {
        ui->logTextEdit->append(tr("Process crash exit"));
        return;
    }
    if (exitCode != 0) {
        ui->logTextEdit->append(tr("Process non zero exit code: %1").arg(exitCode));
        return;
    }
    ui->logTextEdit->append(tr("Process exitted"));
}

void MainWindow::processStateChanged(QProcess::ProcessState state)
{
    if ( ! m_settingsDialog->isAdvancedMode())
        return;

    ui->logTextEdit->append(tr("Process State = %1")
                            .arg(QVariant::fromValue(state).toString()));
}

void MainWindow::processReadyReadStandardError()
{
    const QString txt = QString::fromUtf8(m_process->readAllStandardError());

    captureEvents(txt);

    AdvancedFields af = getAdvancedFields(txt);

    if (m_isScanning)
        m_scanDialog->process(af);

    if ( ! m_settingsDialog->isAdvancedMode())
        return;

    ui->logTextEdit->append(txt);

    updateAdvancedModeFields(af);
}

void MainWindow::processReadyReadStandardOutput()
{
    const QString txt = QString::fromUtf8(m_process->readAllStandardOutput());

    captureEvents(txt);

    AdvancedFields af = getAdvancedFields(txt);

    if (m_isScanning)
        m_scanDialog->process(af);

    if ( ! m_settingsDialog->isAdvancedMode())
        return;

    ui->logTextEdit->append(txt);

    updateAdvancedModeFields(af);
}

void MainWindow::processErrorOccurred(QProcess::ProcessError error)
{
    Q_UNUSED(error);
    ui->logTextEdit->append(tr("ProcessError:%1").arg(m_process->errorString()));
}

void MainWindow::on_actionExit_triggered()
{
    writeSettings();
    qApp->quit();
}

void MainWindow::settingsDialogFinished(int result)
{
    if (result != SettingsDialog::Accepted)
        return;
    stopProcess();
    m_process->setProgram(m_settingsDialog->softfm());
    ui->monoButton->setChecked(m_settingsDialog->isMono());
    radioOn();
    ui->advancedFrame->setVisible(m_settingsDialog->isAdvancedMode());
    m_logDockAction->setVisible(m_settingsDialog->isAdvancedMode());
    ui->logDockWidget->setVisible(m_settingsDialog->isAdvancedMode());
    applyDarkMode();
}

void MainWindow::on_actionSettings_triggered()
{
    m_settingsDialog->open();
}

void MainWindow::on_actionPreview_triggered(bool checked)
{
    if ( ! checked) {
        m_previewTimer->stop();
        return;
    }
    bool ok = false;
    const int sec = QInputDialog::getInt(
                this, tr("Start Preview"), tr("Change Interval as sec"),
                5, 5, INT32_MAX, 1, &ok);
    const int msec = sec * 1000;
    if ( ! ok) {
        ui->actionPreview->setChecked(false);
        return;
    }

    m_previewTimer->setInterval(msec);
    m_previewTimer->start();

    m_previewChanged = QDateTime::currentDateTimeUtc().toMSecsSinceEpoch();
    ui->progressBar->setMaximum(msec);
    m_previewProgressTimer->start(40);
}

void MainWindow::on_actionScan_triggered()
{
    m_isScanning = true;

    QWidget * previewWidget = ui->toolBar->widgetForAction(ui->actionPreview);
    if (previewWidget) {
        QToolButton * previewButton = qobject_cast<QToolButton*>(previewWidget);
        if (previewButton && previewButton->isChecked())
            previewButton->animateClick();
    }
    m_scanDialog->setCrMap(getChannelRecordMap());
    m_scanDialog->open();
}

void MainWindow::scanDialogChangeFreq(int freqAsKhz)
{
    ui->freqDoubleSpinBox->setValue(khz_to_mhz(freqAsKhz));
}

void MainWindow::scanDialogFinished(int result)
{
    m_isScanning = false;
    if (result != QDialog::Accepted)
        return;
    setChannelRecordMap(m_scanDialog->crMap());
    QTimer::singleShot(0, this, &MainWindow::fillTable);
}

void MainWindow::stopProcess(int waitForMsec)
{
    if (m_process->state() != QProcess::NotRunning)
        m_process->terminate();

    m_process->waitForFinished(waitForMsec);

    if (m_process->state() != QProcess::NotRunning)
        m_process->kill();
}

void MainWindow::radioOff()
{
    stopProcess();

    if (m_isOnRequested)
        radioOn();
}

void MainWindow::radioOn()
{
    if ( ! m_isOnRequested)
        return;

    if (isRadioOn()) {
        // postpone until m_process finished
        QTimer::singleShot(100, this, &MainWindow::radioOn);
        return;
    }

    const int freqAsKhz = mhz_to_khz(ui->freqDoubleSpinBox->value());

    const QStringList args = m_settingsDialog->commandArgs(freqAsKhz, m_isScanning);

    m_process->setArguments(args);
    m_process->start();

    ui->logTextEdit->append(QString("> %1 %2")
                            .arg(m_settingsDialog->softfm(), args.join(' '))
                            );
}

void MainWindow::changeFreq()
{
    if ( ! m_isOnRequested)
        return;

    if (isRadioOn())
        radioOff(); // will on after terminate
    else
        radioOn();
}

void MainWindow::on_actionOn_triggered(bool checked)
{
    m_isOnRequested = checked;

    if (checked)
        radioOn();
    else
        radioOff();

    if (m_isOnRequested)
         m_trayIcon->setIcon(QIcon(":/images/tray-icon-playing.svg"));
    else m_trayIcon->setIcon(QIcon(":/images/tray-icon.svg"));

    ui->actionPreview->setEnabled(checked);
    ui->actionScan->setEnabled(checked);
}

void MainWindow::on_freqDoubleSpinBox_valueChanged(double)
{
    const double freqAsMhz = ui->freqDoubleSpinBox->value();
    const int freqAsKhz = mhz_to_khz(freqAsMhz);
    if (m_freqAsKhs == freqAsKhz)
        return;
    m_freqAsKhs = freqAsKhz;

    selectTableRow(m_freqAsKhs);

    const int to = m_isScanning ? 250 : 500;

    m_changeFreqTimer->start(to);
}

void MainWindow::on_freqDownButton_clicked()
{
    ui->freqDoubleSpinBox->stepDown();
}

void MainWindow::on_freqUpButton_clicked()
{
    ui->freqDoubleSpinBox->stepUp();
}

void MainWindow::on_clearButton_clicked()
{
    ui->logTextEdit->clear();
}

void MainWindow::fillTable()
{
    m_crMap = getChannelRecordMap();
    const QList<qint32> freqAsKhzList = m_crMap.keys();
    ui->tableWidget->setRowCount(freqAsKhzList.count());
    int selectedRow = -1;
    for (int i = 0; i < freqAsKhzList.count(); ++i) {
        const qint32 freqAsKhz = freqAsKhzList.at(i);
        const ChannelRecord cr = m_crMap.value(freqAsKhz);
        if (m_freqAsKhs == freqAsKhz)
            selectedRow = i;
        QTableWidgetItem * freqItem = new QTableWidgetItem(TableType_Freq);
        QTableWidgetItem * descItem = new QTableWidgetItem(TableType_Desc);
        QTableWidgetItem * trayItem = new QTableWidgetItem(TableType_Tray);
        TableActionWidget * taw = new TableActionWidget;
        freqItem->setData(Qt::DisplayRole, QVariant::fromValue(freqAsKhz));
        descItem->setText(cr.desc);
        trayItem->setData(Qt::DisplayRole, QVariant::fromValue(cr.showAtTray));
        taw->setFreqAsKhz(freqAsKhz);
        freqItem->setFlags(freqItem->flags() ^ Qt::ItemIsEditable);
        freqItem->setTextAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
        descItem->setTextAlignment(Qt::AlignLeft    | Qt::AlignVCenter);
        trayItem->setTextAlignment(Qt::AlignLeft    | Qt::AlignVCenter);
        ui->tableWidget->setItem(i, Column_Freq, freqItem);
        ui->tableWidget->setItem(i, Column_Desc, descItem);
        ui->tableWidget->setItem(i, Column_Tray, trayItem);
        ui->tableWidget->setCellWidget(i, Column_Act, taw);
        connect(taw, &TableActionWidget::remove, this, &MainWindow::tableActionRemove);
    }
    if (selectedRow >= 0) {
        ui->tableWidget->selectRow(selectedRow);
    }
    else {
        QItemSelectionModel * selection = ui->tableWidget->selectionModel();
        if (selection && selection->hasSelection())
            selection->clear();
    }

    fillTrayIconMenu();
}

void MainWindow::tableActionRemove(int freqAsKhz)
{
    if (m_crMap.contains(freqAsKhz))
        m_crMap.remove(freqAsKhz);
    setChannelRecordMap(m_crMap);
    QTimer::singleShot(0, this, &MainWindow::fillTable);
}

void MainWindow::on_addButton_clicked()
{
    bool ok = false;
    QString desc = QInputDialog::getText(
                this, tr("Desctription"), tr("Desctription"),
                QLineEdit::Normal, QString(), &ok);
    if ( ! ok)
        return;
    const double freqAsMhz = ui->freqDoubleSpinBox->value();
    const int freqAsKhz = mhz_to_khz(freqAsMhz);
    ChannelRecord cr(freqAsKhz, true, desc);
    m_crMap[freqAsKhz] = cr;
    setChannelRecordMap(m_crMap);
    QTimer::singleShot(0, this, &MainWindow::fillTable);
}

void MainWindow::selectTableRow(int freqAsKhz)
{
    QItemSelectionModel * selection = ui->tableWidget->selectionModel();
    if ( ! selection)
        return;
    if (selection->hasSelection())
        selection->clear();

    updateTrayIconActionChecks();

    for (int i = 0; i < ui->tableWidget->rowCount(); ++i) {
        QTableWidgetItem * freqItem = ui->tableWidget->item(i, Column_Freq);
        if ( ! freqItem) continue;
        bool ok = false;
        const int itemFreqAsKhz = freqItem->data(Qt::DisplayRole).toInt(&ok);
        if ( ! ok) continue;
        if (itemFreqAsKhz == freqAsKhz) {
            ui->tableWidget->selectRow(i);
            updateTrayIconActionChecks();
            break;
        }
    }
}

void MainWindow::on_tableWidget_itemSelectionChanged()
{
    QItemSelectionModel * selection = ui->tableWidget->selectionModel();
    if ( ! selection)
        return;
    if ( ! selection->hasSelection())
        return;
    const int row = selection->selectedRows().first().row();
    QTableWidgetItem * freqItem = ui->tableWidget->item(row, Column_Freq);
    if ( ! freqItem)
        return;
    bool ok = false;
    const int itemFreqAsKhz = freqItem->data(Qt::DisplayRole).toInt(&ok);
    if ( ! ok)
        return;
    ui->freqDoubleSpinBox->setValue(khz_to_mhz(itemFreqAsKhz));
}

void MainWindow::on_tableWidget_itemChanged(QTableWidgetItem *item)
{
    if (item->column() < 0 || item->column() >= Column__Max_) {
        return;
    }

    const int row = item->row();

    QTableWidgetItem * freqItem = ui->tableWidget->item(row, Column_Freq);
    if ( ! freqItem) {
        return;
    }
    bool ok = false;
    const int itemFreqAsKhs = freqItem->data(Qt::DisplayRole).toInt(&ok);
    if ( ! ok) {
        return;
    }
    if ( ! m_crMap.contains(itemFreqAsKhs)) {
        return;
    }

    bool changed = false;

    const Columns col = (Columns) item->column();
    switch (col) {
    case Column_Desc: {
        const QString desc = item->data(Qt::DisplayRole).toString();
        if (m_crMap[ itemFreqAsKhs ].desc != desc) {
            m_crMap[ itemFreqAsKhs ].desc = desc;
            changed = true;
        }
        break;
    }
    case Column_Tray: {
        const bool showAtTray = item->data(Qt::DisplayRole).toBool();
        if (m_crMap[ itemFreqAsKhs ].showAtTray != showAtTray) {
            m_crMap[ itemFreqAsKhs ].showAtTray = showAtTray;
            changed = true;
        }
        break;
    }
    default:
        break;
    }

    if (changed) {
        setChannelRecordMap(m_crMap);
        QTimer::singleShot(0, this, &MainWindow::fillTable);
    }
}

void MainWindow::previewTimerTimeout()
{
    if (ui->tableWidget->rowCount() <= 0)
        return;

    int selectedRow = 0;
    QItemSelectionModel * selection = ui->tableWidget->selectionModel();
    if (selection && selection->hasSelection())
        selectedRow = selection->selectedRows().first().row();
    selectedRow++;
    const int nextRow = selectedRow % ui->tableWidget->rowCount();
    QTableWidgetItem * nextItem = ui->tableWidget->item(nextRow, Column_Freq);
    if (nextItem) {
        ui->tableWidget->scrollToItem(nextItem);
    }
    ui->tableWidget->selectRow(nextRow);

    m_previewChanged = QDateTime::currentDateTimeUtc().toMSecsSinceEpoch();
    previewProgressTimerTimeout();
}

void MainWindow::previewProgressTimerTimeout()
{
    const int min = ui->progressBar->minimum();
    const int max = ui->progressBar->maximum();

    if ( ! m_previewTimer->isActive()) {
        m_previewProgressTimer->stop();
        m_previewChanged = -1;
        ui->progressBar->setValue(min);
        return;
    }
    if (m_previewChanged < 0)
        return;
    const qint64 now = QDateTime::currentDateTimeUtc().toMSecsSinceEpoch();
    const qint64 diff = now - m_previewChanged;
    if (diff < 0) {
        ui->progressBar->setValue(0);
        return;
    }
    if (diff >= max) {
        ui->progressBar->setValue(max);
        return;
    }
    ui->progressBar->setValue(diff);
}

void MainWindow::trayIconActivated(QSystemTrayIcon::ActivationReason reason)
{
    switch (reason) {
    case QSystemTrayIcon::Trigger:
    case QSystemTrayIcon::DoubleClick:
        show();
        break;
    case QSystemTrayIcon::MiddleClick: {
        QWidget * onW = ui->toolBar->widgetForAction(ui->actionOn);
        if (onW) {
            QToolButton * onTb = qobject_cast<QToolButton*>(onW);
            if (onTb) {
                onTb->animateClick();
            }
        }
        break;
    }
    default:
        ;
    }
}

void MainWindow::trayIconActionTriggered(bool checked)
{
    Q_UNUSED(checked);
    QAction * a = qobject_cast<QAction*>(sender());
    if ( ! a) return;
    bool ok = false;
    const int freqAsKhz = a->data().toInt(&ok);
    if ( ! ok) return;

    ui->freqDoubleSpinBox->setValue(khz_to_mhz(freqAsKhz));
}

void MainWindow::fillTrayIconMenu()
{
    m_trayIconMenu->clear();

    const QList<qint32> freqAsKhzList = m_crMap.keys();
    for (int i = 0; i < freqAsKhzList.count(); ++i) {
        const qint32 freqAsKhz = freqAsKhzList.at(i);
        const ChannelRecord cr = m_crMap.value(freqAsKhz);
        if ( ! cr.showAtTray) continue;
        QAction * a = m_trayIconMenu->addAction(QString("%1 : %2")
                                                .arg(khz_to_mhz(freqAsKhz), 0, 'f', 1)
                                                .arg(cr.desc));
        a->setCheckable(true);
        a->setData(QVariant::fromValue(freqAsKhz));
        connect(a, &QAction::triggered, this, &MainWindow::trayIconActionTriggered);
    }

    m_trayIconMenu->addSeparator();

    QAction * exitAction = m_trayIconMenu->addAction(tr("Exit"));
    connect(exitAction, &QAction::triggered, qApp, &QCoreApplication::quit);

    updateTrayIconActionChecks();
}

void MainWindow::updateTrayIconActionChecks()
{
    int row = -1;
    int freqAsKhz = -1;
    QItemSelectionModel * selection = ui->tableWidget->selectionModel();
    if (selection->hasSelection()) {
        row = selection->selectedRows().first().row();
        QTableWidgetItem * freqItem = ui->tableWidget->item(row, Column_Freq);
        if (freqItem) {
            bool ok = false;
            const int itemFreqAsKhz = freqItem->data(Qt::DisplayRole).toInt(&ok);
            if (ok)
                freqAsKhz = itemFreqAsKhz;
        }
    }

    QList<QAction*> list = m_trayIconMenu->actions();
    for (int i = 0; i < list.count(); ++i) {
        QAction * a = list.at(i);
        if ( ! a->isCheckable()) continue;
        a->setChecked(a->data().toInt() == freqAsKhz);
    }
}

void MainWindow::on_monoButton_toggled(bool checked)
{
    if (checked)
         ui->monoButton->setIcon(QIcon(":/images/mono.svg"));
    else ui->monoButton->setIcon(QIcon(":/images/stereo.svg"));

    m_settingsDialog->setIsMono(checked);
    changeFreq();
}

void MainWindow::applyDarkMode()
{
    // qDebug() << Q_FUNC_INFO << checked;

    if (m_settingsDialog->isDarkMode()) {
        QFile qdarkstyleFile(":qdarkstyle/style.qss");
        if ( ! qdarkstyleFile.exists()) {
            qDebug("Unable to set QDarkStyleSheet stylesheet, file not found\n");
        }
        else {
            qdarkstyleFile.open(QFile::ReadOnly | QFile::Text);
            QTextStream ts(&qdarkstyleFile);
            qApp->setStyleSheet(ts.readAll());
        }
    }
    else {
        // QApplication::setStyle(QStyleFactory::create("Fusion"));
        qApp->setStyleSheet("");
    }
}

bool MainWindow::isRadioOn() const
{
    return m_process->state() != QProcess::NotRunning;
}

MainWindow::AdvancedFields MainWindow::getAdvancedFields(const QString &txt) const
{
    AdvancedFields res;

    static QLatin1String strBlk ("blk="  );
    static QLatin1String strFreq("freq=" );
    static QLatin1String strPpm ("ppm="  );
    static QLatin1String strIf  ("IF="   );
    static QLatin1String strBb  ("BB="   );
    static QLatin1String strAud ("audio=");
    static QLatin1String strBuf ("buf="  );

    static QLatin1String strMhz ("MHz");
    static QLatin1String strDb  ("dB");
    static QLatin1String strSec ("s");

    QStringRef ref(&txt);

    const int idxBlk  = ref.indexOf(strBlk , 0, Qt::CaseSensitive);
    const int idxFreq = ref.indexOf(strFreq, 0, Qt::CaseSensitive);
    const int idxPpm  = ref.indexOf(strPpm , 0, Qt::CaseSensitive);
    const int idxIf   = ref.indexOf(strIf  , 0, Qt::CaseSensitive);
    const int idxBb   = ref.indexOf(strBb  , 0, Qt::CaseSensitive);
    const int idxAud  = ref.indexOf(strAud , 0, Qt::CaseSensitive);
    const int idxBuf  = ref.indexOf(strBuf , 0, Qt::CaseSensitive);

    const int idxMhz  = ref.indexOf(strMhz , idxFreq, Qt::CaseSensitive);
    const int idxIfDb = ref.indexOf(strDb,   idxIf,   Qt::CaseSensitive);
    const int idxBbDb = ref.indexOf(strDb,   idxBb,   Qt::CaseSensitive);
    const int idxAuDb = ref.indexOf(strDb,   idxAud,  Qt::CaseSensitive);
    const int idxSec  = ref.indexOf(strSec,  idxBuf,  Qt::CaseSensitive);

    if (idxBlk  < 0) return res;
    if (idxFreq < 0) return res;
    if (idxPpm  < 0) return res;
    if (idxIf   < 0) return res;
    if (idxBb   < 0) return res;
    if (idxAud  < 0) return res;
    if (idxBuf  < 0) return res;

    if (idxMhz  < 0) return res;
    if (idxIfDb < 0) return res;
    if (idxBbDb < 0) return res;
    if (idxAuDb < 0) return res;
    if (idxSec  < 0) return res;

    bool okBlk  = false;
    bool okFreq = false;
    bool okPpm  = false;
    bool okIf   = false;
    bool okBb   = false;
    bool okAud  = false;
    bool okBuf  = false;

    res.blk         = ref.mid(idxBlk  + strBlk .size(), idxFreq - idxBlk  - strBlk .size()).toDouble(&okBlk );
    res.freqAsMhz   = ref.mid(idxFreq + strFreq.size(), idxMhz  - idxFreq - strFreq.size()).toDouble(&okFreq);
    res.ppm         = ref.mid(idxPpm  + strPpm .size(), idxIf   - idxPpm  - strPpm .size()).toDouble(&okPpm );
    res.if_         = ref.mid(idxIf   + strIf  .size(), idxIfDb - idxIf   - strIf  .size()).toDouble(&okIf  );
    res.bb          = ref.mid(idxBb   + strBb  .size(), idxBbDb - idxBb   - strBb  .size()).toDouble(&okBb  );
    res.audio       = ref.mid(idxAud  + strAud .size(), idxAuDb - idxAud  - strAud .size()).toDouble(&okAud );
    res.buf         = ref.mid(idxBuf  + strBuf .size(), idxSec  - idxBuf  - strBuf .size()).toDouble(&okBuf );

    res.isValid =
            okBlk  &&
            okFreq &&
            okPpm  &&
            okIf   &&
            okBb   &&
            okAud  &&
            okBuf;

    return res;
}

void MainWindow::updateAdvancedModeFields(const AdvancedFields & af)
{
    ui->advFreqDoubleSpinBox->setValue(     af.isValid ? af.freqAsMhz : 0.0);
    ui->advPpmDoubleSpinBox->setValue(      af.isValid ? af.ppm       : 0.0);
    ui->advIfDoubleSpinBox->setValue(       af.isValid ? af.if_       : 0.0);
    ui->advBbDoubleSpinBox->setValue(       af.isValid ? af.bb        : 0.0);
    ui->advAudioDoubleSpinBox->setValue(    af.isValid ? af.audio     : 0.0);
    ui->advBufDoubleSpinBox->setValue(      af.isValid ? af.buf       : 0.0);
}

void MainWindow::captureEvents(const QString &txt)
{
//    qDebug() << Q_FUNC_INFO << txt;

    if (txt.contains("SoftFM")) {
        // started
        m_stereoLabel->setText("?");
    }
    else if (txt.contains("got stereo signal")) {
        m_stereoLabel->setText("Stereo");
    }
    else if (txt.contains("no/lost stereo signal")) {
        m_stereoLabel->setText("Mono");
    }
}

ChannelRecordMap MainWindow::getChannelRecordMap() const
{
    ChannelRecordMap cm = QSettings().value("MainWindow/channelRecordMap")
            .value<ChannelRecordMap>();

    // upgrade records, if have any old version
    const QList<qint32> freqAsKhzList = cm.keys();
    for (int i = 0; i < freqAsKhzList.count(); ++i) {
        const qint32 freqAsKhz = freqAsKhzList.at(i);
        if (cm[freqAsKhz].version == 1) {
            cm[freqAsKhz].version = 2;
            // freqAsMhz and freqAsKhz will be filled at stream function
        }
    }

    return cm;
}

void MainWindow::setChannelRecordMap(const ChannelRecordMap &crMap)
{
    QSettings().setValue("MainWindow/channelRecordMap", QVariant::fromValue(crMap));
}
