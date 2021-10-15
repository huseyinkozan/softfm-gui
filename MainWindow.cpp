#include "MainWindow.h"
#include "ui_MainWindow.h"

#include <QCloseEvent>
#include <QDebug>
#include <QInputDialog>
#include <QLabel>
#include <QSettings>
#include <QTextStream>
#include <QTimer>

#include "SettingsDialog.h"
#include "TableActionWidget.h"


enum Columns {
    Column_Freq = 0,
    Column_Desc,
    Column_Act,
    Column__Max_
};

enum TableTypes {
    TableType_Freq = QTableWidgetItem::UserType +  1,
    TableType_Desc = QTableWidgetItem::UserType +  2,
    TableType_Act  = QTableWidgetItem::UserType +  3,
};

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    QAction * logDockAction = ui->logDockWidget->toggleViewAction();
    logDockAction->setObjectName("logDockAction");
    logDockAction->setIcon(QIcon(":images/log.svg"));
    logDockAction->setStatusTip(tr("Show Log"));
    ui->toolBar->insertAction(ui->actionExit, logDockAction);

    QWidget * exp1 = new QWidget(this);
    exp1->setObjectName("exp1");
    exp1->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Expanding);
    ui->toolBar->insertWidget(ui->actionSettings, exp1);

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

    m_freq = ui->freqDoubleSpinBox->value();

    QStringList headerList = QStringList()
            << tr("Freq")
            << tr("Desc")
            << tr("Act")
               ;
    Q_ASSERT(headerList.count() == Column__Max_);
    ui->tableWidget->setColumnCount(headerList.count());
    ui->tableWidget->setHorizontalHeaderLabels(headerList);
    QHeaderView * hv = ui->tableWidget->horizontalHeader();
    Q_CHECK_PTR(hv);
    hv->setSectionResizeMode(Column_Freq, QHeaderView::Interactive);
    hv->setSectionResizeMode(Column_Desc, QHeaderView::Stretch);
    hv->setSectionResizeMode(Column_Act,  QHeaderView::Fixed);
    hv->resizeSection(Column_Act, 40);
    ui->tableWidget->horizontalHeaderItem(Column_Freq)->setStatusTip(tr("Frequency"));
    ui->tableWidget->horizontalHeaderItem(Column_Desc)->setStatusTip(tr("Description"));
    ui->tableWidget->horizontalHeaderItem(Column_Act )->setStatusTip(tr("Action"));

    m_process = new QProcess(this);
//    m_process->setProcessChannelMode(QProcess::MergedChannels);
    connect(m_process, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished), this, &MainWindow::processFinished);
    connect(m_process, &QProcess::stateChanged,                                       this, &MainWindow::processStateChanged);
    connect(m_process, &QProcess::readyReadStandardError,                             this, &MainWindow::processReadyReadStandardError);
    connect(m_process, &QProcess::readyReadStandardOutput,                            this, &MainWindow::processReadyReadStandardOutput);
    connect(m_process, &QProcess::errorOccurred,                                      this, &MainWindow::processErrorOccurred);

    m_settingsDialog = new SettingsDialog(this);
    m_settingsDialog->setObjectName("m_settingsDialog");
    connect(m_settingsDialog, &SettingsDialog::finished, this, &MainWindow::settingsDialogFinished);

    QAction * exitAction = new QAction(tr("&Exit"), this);
    connect(exitAction, &QAction::triggered, qApp, &QCoreApplication::quit);
    QMenu * trayIconMenu = new QMenu(this);
    trayIconMenu->addAction(exitAction);
    m_trayIcon = new QSystemTrayIcon(this);
    m_trayIcon->setContextMenu(trayIconMenu);
    m_trayIcon->setIcon(QIcon(":/images/tray-icon.svg"));
    m_trayIcon->show();
    connect(m_trayIcon, &QSystemTrayIcon::activated, this, &MainWindow::trayIconActivated);

    m_changeFreqTimer = new QTimer(this);
    m_changeFreqTimer->setSingleShot(true);
    connect(m_changeFreqTimer, &QTimer::timeout, this, &MainWindow::changeFreq);

    readSettings();

    QTimer::singleShot(0, this, [this]{
        settingsDialogFinished(SettingsDialog::Accepted);
        fillTable();
    });
}

MainWindow::~MainWindow()
{
    m_isOnRequested = false;
    radioOff();

    m_process->waitForFinished(3000);

    if (m_process->state() != QProcess::NotRunning)
        m_process->kill();

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

    if ( ! m_settingsDialog->isAdvancedMode())
        return;

    ui->logTextEdit->append(txt);

    updateAdvancedModeFields(txt);
}

void MainWindow::processReadyReadStandardOutput()
{
    const QString txt = QString::fromUtf8(m_process->readAllStandardOutput());

    captureEvents(txt);

    if ( ! m_settingsDialog->isAdvancedMode())
        return;

    ui->logTextEdit->append(txt);

    updateAdvancedModeFields(txt);
}

void MainWindow::processErrorOccurred(QProcess::ProcessError error)
{
    Q_UNUSED(error);
    ui->logTextEdit->append(tr("ProcessError:%1").arg(m_process->errorString()));
}

void MainWindow::on_actionExit_triggered()
{
    qApp->quit();
}

void MainWindow::settingsDialogFinished(int result)
{
    if (result != SettingsDialog::Accepted)
        return;
    m_process->setProgram(m_settingsDialog->softfm());
    ui->advancedFrame->setVisible(m_settingsDialog->isAdvancedMode());
    applyDarkMode();
    ui->stereoButton->setChecked(m_settingsDialog->isStereo());
    radioOn();
}

void MainWindow::on_actionSettings_triggered()
{
    m_settingsDialog->open();
}

void MainWindow::radioOff()
{
    if (m_process->state() != QProcess::NotRunning)
        m_process->terminate();

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

    const double freq = ui->freqDoubleSpinBox->value();

    const QStringList args = m_settingsDialog->commandArgs(freq);

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
}

void MainWindow::on_freqDoubleSpinBox_valueChanged(double)
{
    const double freq = ui->freqDoubleSpinBox->value();
    if (m_freq == freq)
        return;
    m_freq = freq;

    selectTableRow(m_freq);

    m_changeFreqTimer->start(500);
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
    const FreqDescMap fdMap = tableData();
    const QList<double> freqs = fdMap.keys();
    ui->tableWidget->setRowCount(freqs.count());
    int selectedRow = -1;
    for (int i = 0; i < freqs.count(); ++i) {
        const double freq = freqs.at(i);
        const QString desc = fdMap.value(freq);
        if (qFuzzyCompare(m_freq, freq))
            selectedRow = i;
        QTableWidgetItem * freqItem = new QTableWidgetItem(TableType_Freq);
        QTableWidgetItem * descItem = new QTableWidgetItem(TableType_Desc);
        TableActionWidget * taw = new TableActionWidget;
        freqItem->setData(Qt::DisplayRole, QVariant::fromValue(freq));
        descItem->setText(desc);
        taw->setFreq(freq);
        freqItem->setTextAlignment(Qt::AlignLeft | Qt::AlignVCenter);
        descItem->setTextAlignment(Qt::AlignLeft | Qt::AlignVCenter);
        ui->tableWidget->setItem(i, Column_Freq, freqItem);
        ui->tableWidget->setItem(i, Column_Desc, descItem);
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
}

void MainWindow::tableActionRemove(double freq)
{
    FreqDescMap fdMap = tableData();
    if (fdMap.contains(freq))
        fdMap.remove(freq);
    QSettings().setValue("MainWindow/freqDescMap", QVariant::fromValue(fdMap));
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
    const double freq = ui->freqDoubleSpinBox->value();
    FreqDescMap fdMap = tableData();
    fdMap[freq] = desc;
    QSettings().setValue("MainWindow/freqDescMap", QVariant::fromValue(fdMap));
    QTimer::singleShot(0, this, &MainWindow::fillTable);
}

void MainWindow::selectTableRow(double freq)
{
    QItemSelectionModel * selection = ui->tableWidget->selectionModel();
    if ( ! selection)
        return;
    if (selection->hasSelection())
        selection->clear();

    for (int i = 0; i < ui->tableWidget->rowCount(); ++i) {
        QTableWidgetItem * freqItem = ui->tableWidget->item(i, Column_Freq);
        if ( ! freqItem) continue;
        bool ok = false;
        const double itemFreq = freqItem->data(Qt::DisplayRole).toDouble(&ok);
        if ( ! ok) continue;
        if (qFuzzyCompare(itemFreq, freq)) {
            ui->tableWidget->selectRow(i);
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
    const double itemFreq = freqItem->data(Qt::DisplayRole).toDouble(&ok);
    if ( ! ok)
        return;
    ui->freqDoubleSpinBox->setValue(itemFreq);
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

void MainWindow::updateAdvancedModeFields(const QString &txt)
{
    QStringRef ref = QStringRef(&txt).trimmed();

    QLatin1String freqBeginStr("freq=" ), freqEndStr("MHz");
    QLatin1String ppmBeginStr( "ppm="  ), ppmEndStr("IF=");
    QLatin1String bbBeginStr(  "BB="   ), bbEndStr("dB");
    QLatin1String audBeginStr( "audio="), audEndStr("dB");
    QLatin1String bufBeginStr( "buf="  ), bufEndStr("s");
    int freqBegin = ref.indexOf(freqBeginStr);
    int freqEnd   = ref.indexOf(freqEndStr, freqBegin);
    int ppmBegin  = ref.indexOf(ppmBeginStr);
    int ppmEnd    = ref.indexOf(ppmEndStr,  ppmBegin);
    int bbBegin   = ref.indexOf(bbBeginStr);
    int bbEnd     = ref.indexOf(bbEndStr,   bbBegin);
    int audBegin  = ref.indexOf(audBeginStr);
    int audEnd    = ref.indexOf(audEndStr,  audBegin);
    int bufBegin  = ref.indexOf(bufBeginStr);
    int bufEnd    = ref.indexOf(bufEndStr,  bufBegin);
    if (freqBegin < 0 || freqEnd < 0) return;
    if (ppmBegin  < 0 || ppmEnd  < 0) return;
    if (bbBegin   < 0 || bbEnd   < 0) return;
    if (audBegin  < 0 || audEnd  < 0) return;
    if (bufBegin  < 0 || bufEnd  < 0) return;
    freqBegin += freqBeginStr.size();
    ppmBegin  += ppmBeginStr.size();
    bbBegin   += bbBeginStr.size();
    audBegin  += audBeginStr.size();
    bufBegin  += bufBeginStr.size();
    const double freq = ref.mid(freqBegin, freqEnd - freqBegin).trimmed().toDouble();
    const double ppm  = ref.mid(ppmBegin,  ppmEnd  - ppmBegin ).trimmed().toDouble();
    const double bb   = ref.mid(bbBegin,   bbEnd   - bbBegin  ).trimmed().toDouble();
    const double aud  = ref.mid(audBegin,  audEnd  - audBegin ).trimmed().toDouble();
    const double buf  = ref.mid(bufBegin,  bufEnd  - bufBegin ).trimmed().toDouble();
    ui->advFreqDoubleSpinBox->setValue(freq);
    ui->advPpmDoubleSpinBox->setValue(ppm);
    ui->advBbDoubleSpinBox->setValue(bb);
    ui->advAudioDoubleSpinBox->setValue(aud);
    ui->advBufDoubleSpinBox->setValue(buf);
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

FreqDescMap MainWindow::tableData() const
{
    return QSettings().value("MainWindow/freqDescMap").value<FreqDescMap>();
}

void MainWindow::on_stereoButton_toggled(bool checked)
{
    if (checked)
         ui->stereoButton->setIcon(QIcon(":/images/stereo.svg"));
    else ui->stereoButton->setIcon(QIcon(":/images/mono.svg"));

    m_settingsDialog->setIsStereo(checked);
    changeFreq();
}

