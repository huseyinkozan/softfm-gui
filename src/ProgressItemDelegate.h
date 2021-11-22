#ifndef PROGRESSITEMDELEGATE_H
#define PROGRESSITEMDELEGATE_H

#include <QStyledItemDelegate>
#include <QProgressBar>
#include <QPainter>


class ProgressItemDelegate : public QStyledItemDelegate
{
public:
    ProgressItemDelegate(QObject * parent) : QStyledItemDelegate(parent)
    {
    }

    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
    {
        QProgressBar pb;
        const QAbstractItemModel * model = index.model();
        if ( ! model)
            return QStyledItemDelegate::paint(painter, option, index);
        bool ok = false;
        const int percent = model->data(index, Qt::DisplayRole).toInt(&ok);
        if ( ! ok)
            return QStyledItemDelegate::paint(painter, option, index);

        pb.resize(option.rect.size());
        pb.setMinimum(0);
        pb.setMaximum(100);
        pb.setValue(percent);
        pb.setTextVisible(false);

        painter->save();
        painter->translate(option.rect.topLeft());
        pb.render(painter);
        painter->restore();
    }
};


#endif // PROGRESSITEMDELEGATE_H
