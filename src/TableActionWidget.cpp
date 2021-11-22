#include "TableActionWidget.h"
#include "ui_TableActionWidget.h"

TableActionWidget::TableActionWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::TableActionWidget)
{
    ui->setupUi(this);
}

TableActionWidget::~TableActionWidget()
{
    delete ui;
}

int TableActionWidget::freqAsKhz() const
{
    return m_freqAsKhz;
}

void TableActionWidget::setFreqAsKhz(int freqAsKhz)
{
    m_freqAsKhz = freqAsKhz;
}

void TableActionWidget::on_removeButton_clicked()
{
    emit remove(m_freqAsKhz);
}

