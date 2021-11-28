#include "ScanDialog.h"
#include "ui_ScanDialog.h"

#include <QDebug>
#include <QTimer>
#include <cmath>

#include "CheckBoxDelegate.h"
#include "FreqItemDelegate.h"
#include "ProgressItemDelegate.h"


enum Columns {
    Column_Freq = 0,
    Column_Desc,
    Column_Tray,
    Column_List,
    Column_Prog,
    Column__Max_
};

enum TableTypes {
    TableType_Freq = QTableWidgetItem::UserType +  1,
    TableType_Desc = QTableWidgetItem::UserType +  2,
    TableType_Tray = QTableWidgetItem::UserType +  3,
    TableType_List = QTableWidgetItem::UserType +  4,
    TableType_Prog = QTableWidgetItem::UserType +  5,
};



ScanDialog::ScanDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ScanDialog)
{
    ui->setupUi(this);

    QStringList headerList = QStringList()
            << tr("Freq")
            << tr("Desc")
            << tr("Tray")
            << tr("List")
            << tr("IF")
               ;
    Q_ASSERT(headerList.count() == Column__Max_);
    ui->tableWidget->setColumnCount(headerList.count());
    ui->tableWidget->setHorizontalHeaderLabels(headerList);
    QHeaderView * hv = ui->tableWidget->horizontalHeader();
    Q_CHECK_PTR(hv);
    hv->setSectionResizeMode(Column_Freq, QHeaderView::Interactive);
    hv->setSectionResizeMode(Column_Desc, QHeaderView::Interactive);
    hv->setSectionResizeMode(Column_Tray, QHeaderView::Fixed);
    hv->setSectionResizeMode(Column_List, QHeaderView::Fixed);
    hv->setSectionResizeMode(Column_Prog, QHeaderView::Stretch);
    hv->resizeSection(Column_Freq, 40);
    hv->resizeSection(Column_Tray, 40);
    hv->resizeSection(Column_List, 40);

    FreqItemDelegate * freqColItemDlg = new FreqItemDelegate(this);
    ui->tableWidget->setItemDelegateForColumn(Column_Freq, freqColItemDlg);

    CheckBoxDelegate * trayColItemDlg = new CheckBoxDelegate(this);
    trayColItemDlg->setLabelText(tr("Show"));
    ui->tableWidget->setItemDelegateForColumn(Column_Tray, trayColItemDlg);

    CheckBoxDelegate * listColItemDlg = new CheckBoxDelegate(this);
    listColItemDlg->setLabelText(tr("Save"));
    ui->tableWidget->setItemDelegateForColumn(Column_List, listColItemDlg);

    ProgressItemDelegate * progColItemDlg = new ProgressItemDelegate(this);
    ui->tableWidget->setItemDelegateForColumn(Column_Prog, progColItemDlg);

    int freqAsKhz = 87000;
    while (freqAsKhz <= 108000) {
        QTableWidgetItem * freqItem = new QTableWidgetItem(TableType_Freq);
        QTableWidgetItem * descItem = new QTableWidgetItem(TableType_Desc);
        QTableWidgetItem * trayItem = new QTableWidgetItem(TableType_Tray);
        QTableWidgetItem * listItem = new QTableWidgetItem(TableType_List);
        QTableWidgetItem * progItem = new QTableWidgetItem(TableType_Prog);
        freqItem->setData(Qt::DisplayRole, QVariant::fromValue(freqAsKhz));
        descItem->setData(Qt::DisplayRole, QVariant::fromValue(QString()));
        trayItem->setData(Qt::DisplayRole, QVariant::fromValue(false));
        listItem->setData(Qt::DisplayRole, QVariant::fromValue(false));
        progItem->setData(Qt::DisplayRole, QVariant::fromValue((int) 0));
        freqItem->setFlags(freqItem->flags() ^ Qt::ItemIsEditable);
        progItem->setFlags(freqItem->flags() ^ Qt::ItemIsEditable);
        freqItem->setTextAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
        descItem->setTextAlignment(Qt::AlignLeft    | Qt::AlignVCenter);
        trayItem->setTextAlignment(Qt::AlignLeft    | Qt::AlignVCenter);
        listItem->setTextAlignment(Qt::AlignLeft    | Qt::AlignVCenter);
        progItem->setTextAlignment(Qt::AlignLeft    | Qt::AlignVCenter);

        const int row = ui->tableWidget->rowCount();
        ui->tableWidget->setRowCount(row + 1);
        ui->tableWidget->setItem(row, Column_Freq, freqItem);
        ui->tableWidget->setItem(row, Column_Desc, descItem);
        ui->tableWidget->setItem(row, Column_Tray, trayItem);
        ui->tableWidget->setItem(row, Column_List, listItem);
        ui->tableWidget->setItem(row, Column_Prog, progItem);
        freqAsKhz += 100;
    }

    m_timer = new QTimer(this);
    m_timer->setObjectName("m_timer");
    m_timer->setSingleShot(false);
    connect(m_timer, &QTimer::timeout, this, &ScanDialog::timerTimeout);
}

ScanDialog::~ScanDialog()
{
    delete ui;
}

const ChannelRecordMap &ScanDialog::crMap() const
{
    return m_crMap;
}

void ScanDialog::setCrMap(const ChannelRecordMap &newCrMap)
{
    m_crMap = newCrMap;
}

void ScanDialog::process(const MainWindow::AdvancedFields &af)
{
    if ( ! af.isValid)
        return;

    QItemSelectionModel * selection = ui->tableWidget->selectionModel();
    if ( ! selection)
        return;
    if ( ! selection->hasSelection())
        return;
    const int row = selection->selectedRows().first().row();
    QTableWidgetItem * freqItem = ui->tableWidget->item(row, Column_Freq);
    QTableWidgetItem * progItem = ui->tableWidget->item(row, Column_Prog);
    if ( ! freqItem) return;
    if ( ! progItem) return;
    const int percent = abs((int)af.if_);
    progItem->setData(Qt::DisplayRole, QVariant::fromValue((int) percent));
}

bool ScanDialog::isMuted() const
{
    return ui->mutedCheckBox->isChecked();
}

void ScanDialog::done(int result)
{
    if (ui->scanButton->isChecked())
        ui->scanButton->animateClick();

    QDialog::done(result);
}

void ScanDialog::showEvent(QShowEvent *event)
{
    const int rowCount = ui->tableWidget->rowCount();
    for (int row = 0; row < rowCount; ++row) {
        QTableWidgetItem * freqItem = ui->tableWidget->item(row, Column_Freq);
        QTableWidgetItem * descItem = ui->tableWidget->item(row, Column_Desc);
        QTableWidgetItem * trayItem = ui->tableWidget->item(row, Column_Tray);
        QTableWidgetItem * listItem = ui->tableWidget->item(row, Column_List);
        QTableWidgetItem * progItem = ui->tableWidget->item(row, Column_Prog);
        if ( ! freqItem) continue;
        if ( ! descItem) continue;
        if ( ! trayItem) continue;
        if ( ! listItem) continue;
        if ( ! progItem) continue;

        // clear
        descItem->setData(Qt::DisplayRole, QVariant::fromValue(QString()));
        trayItem->setData(Qt::DisplayRole, QVariant::fromValue(false));
        listItem->setData(Qt::DisplayRole, QVariant::fromValue(false));
        progItem->setData(Qt::DisplayRole, QVariant::fromValue((int) 0));

        // fill from m_crMap
        bool ok = false;
        const int freqAsKhz = freqItem->data(Qt::DisplayRole).toInt(&ok);
        if ( ! ok) continue;
        if ( ! m_crMap.contains(freqAsKhz)) continue;
        const ChannelRecord cr = m_crMap.value(freqAsKhz);
        descItem->setData(Qt::DisplayRole, QVariant::fromValue(cr.desc));
        trayItem->setData(Qt::DisplayRole, QVariant::fromValue(cr.showAtTray));
        listItem->setData(Qt::DisplayRole, QVariant::fromValue(true));
    }

    ui->tableWidget->clearSelection();

    QDialog::showEvent(event);
}

void ScanDialog::timerTimeout()
{
    const int row = ui->tableWidget->currentRow();
    const int nextRow = row + 1;
    if (nextRow >= ui->tableWidget->rowCount()) {
        if (ui->scanButton->isChecked())
            ui->scanButton->animateClick();
    }
    ui->tableWidget->selectRow(nextRow);
}

void ScanDialog::on_scanButton_clicked()
{
    const bool checked = ui->scanButton->isChecked();
    const int interval = ui->intervalSpinBox->value();

    ui->tableWidget->setEnabled( ! checked);
    ui->intervalSpinBox->setEnabled( ! checked);

    if (checked) {
        m_timer->setInterval(interval * 1000);
        m_timer->start();
        ui->tableWidget->sortByColumn(Column_Freq, Qt::AscendingOrder);
        ui->tableWidget->selectRow(0);
    }
    else {
        m_timer->stop();
    }
}

void ScanDialog::on_okButton_clicked()
{
    ChannelRecordMap crMap;

    const int rowCount = ui->tableWidget->rowCount();
    for (int row = 0; row < rowCount; ++row) {
        QTableWidgetItem * freqItem = ui->tableWidget->item(row, Column_Freq);
        QTableWidgetItem * descItem = ui->tableWidget->item(row, Column_Desc);
        QTableWidgetItem * trayItem = ui->tableWidget->item(row, Column_Tray);
        QTableWidgetItem * listItem = ui->tableWidget->item(row, Column_List);
        if ( ! freqItem) continue;
        if ( ! descItem) continue;
        if ( ! trayItem) continue;
        if ( ! listItem) continue;
        const bool listed = listItem->data(Qt::DisplayRole).toBool();
        if ( ! listed) continue;
        bool ok = false;
        const int freqAsKhz = freqItem->data(Qt::DisplayRole).toInt(&ok);
        if ( ! ok) continue;
        const QString desc = descItem->data(Qt::DisplayRole).toString();
        const bool    tray = trayItem->data(Qt::DisplayRole).toBool();
        ChannelRecord cr(freqAsKhz, tray, desc);
        crMap[freqAsKhz] = cr;
    }

    m_crMap = crMap;

    accept();
}

void ScanDialog::on_cancelButton_clicked()
{
    reject();
}

void ScanDialog::on_tableWidget_itemSelectionChanged()
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
    const int freqAsKhz = freqItem->data(Qt::DisplayRole).toInt(&ok);
    if ( ! ok)
        return;
    emit changeFreq(freqAsKhz);
}

