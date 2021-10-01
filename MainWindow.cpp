#include "MainWindow.h"
#include "ui_MainWindow.h"

#include <QCloseEvent>
#include <QLabel>
#include <QSettings>
#include <QTimer>

#include "SettingsDialog.h"


MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    QAction * logDockAction = ui->logDockWidget->toggleViewAction();
    logDockAction->setObjectName("logDockAction");
    logDockAction->setIcon(QIcon(":images/console.svg"));
    logDockAction->setStatusTip(tr("Show Log"));
    ui->toolBar->insertAction(ui->actionSettings, logDockAction);

    QWidget * exp1 = new QWidget(this);
    exp1->setObjectName("exp1");
    exp1->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Expanding);
    ui->toolBar->insertWidget(ui->actionSettings, exp1);

    m_versionLabel = new QLabel(tr("%1").arg(qApp->applicationVersion()));
    m_versionLabel->setObjectName("m_versionLabel");
    m_versionLabel->setEnabled(false);
    m_versionLabel->setStatusTip(tr("Version"));
    statusBar()->addPermanentWidget(m_versionLabel);

    m_process = new QProcess(this);
//    m_process->setProcessChannelMode(QProcess::MergedChannels);
    connect(m_process, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),   this, &MainWindow::processFinished);
    connect(m_process, &QProcess::stateChanged,                                         this, &MainWindow::processStateChanged);
    connect(m_process, &QProcess::readyReadStandardError,                               this, &MainWindow::processReadyReadStandardError);
    connect(m_process, &QProcess::readyReadStandardOutput,                              this, &MainWindow::processReadyReadStandardOutput);
    connect(m_process, &QProcess::errorOccurred,                                        this, &MainWindow::processErrorOccurred);

    m_settingsDialog = new SettingsDialog(this);
    m_settingsDialog->setObjectName("m_settingsDialog");
    connect(m_settingsDialog, &SettingsDialog::finished, this, &MainWindow::settingsDialogFinished);

    handleAdvancedModeChange();

    readSettings();

    QTimer::singleShot(0, this, [this]{
        settingsDialogFinished(SettingsDialog::Accepted);
    });
}

MainWindow::~MainWindow()
{
    delete ui;
}

bool MainWindow::isAdvancedMode() const
{
    return m_isAdvancedMode;
}

void MainWindow::setIsAdvancedMode(bool newIsAdvancedMode)
{
    m_isAdvancedMode = newIsAdvancedMode;
    handleAdvancedModeChange();
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

}

void MainWindow::processStateChanged(QProcess::ProcessState state)
{

}

void MainWindow::processReadyReadStandardError()
{

}

void MainWindow::processReadyReadStandardOutput()
{

}

void MainWindow::processErrorOccurred(QProcess::ProcessError error)
{

}

void MainWindow::settingsDialogFinished(int result)
{
    if (result != SettingsDialog::Accepted)
        return;
    m_process->setProgram(m_settingsDialog->softfm());
}

void MainWindow::handleAdvancedModeChange()
{
    // TODO
}


void MainWindow::on_actionSettings_triggered()
{
    m_settingsDialog->open();
}

