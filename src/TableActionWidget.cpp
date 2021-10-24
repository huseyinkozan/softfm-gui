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

double TableActionWidget::freq() const
{
    return m_freq;
}

void TableActionWidget::setFreq(double newFreq)
{
    m_freq = newFreq;
}

void TableActionWidget::on_removeButton_clicked()
{
    emit remove(m_freq);
}

