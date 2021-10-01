#include "MainWindow.h"
#include "ui_MainWindow.h"

#include <QLabel>


MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    m_versionLabel = new QLabel(tr("%1").arg(qApp->applicationVersion()));
    m_versionLabel->setObjectName("m_versionLabel");
    m_versionLabel->setEnabled(false);
    m_versionLabel->setStatusTip(tr("Version"));
    statusBar()->addPermanentWidget(m_versionLabel);

    handleAdvancedModeChange();
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

void MainWindow::handleAdvancedModeChange()
{
    // TODO
}

