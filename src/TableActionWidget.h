#ifndef TABLEACTIONWIDGET_H
#define TABLEACTIONWIDGET_H

#include <QWidget>

namespace Ui {
class TableActionWidget;
}

class TableActionWidget : public QWidget
{
    Q_OBJECT

public:
    explicit TableActionWidget(QWidget *parent = nullptr);
    ~TableActionWidget();

    int freqAsKhz() const;
    void setFreqAsKhz(int freqAsKhz);

signals:
    void remove(int freqAsKhz);

private slots:
    void on_removeButton_clicked();

private:
    Ui::TableActionWidget *ui;
    int m_freqAsKhz = 0.0;
};

#endif // TABLEACTIONWIDGET_H
