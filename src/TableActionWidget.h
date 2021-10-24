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

    double freq() const;
    void setFreq(double newFreq);

signals:
    void remove(double freq);

private slots:
    void on_removeButton_clicked();

private:
    Ui::TableActionWidget *ui;
    double m_freq = 0.0;
};

#endif // TABLEACTIONWIDGET_H
