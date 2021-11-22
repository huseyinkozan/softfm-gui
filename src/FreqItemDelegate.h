#ifndef FREQITEMDELEGATE_H
#define FREQITEMDELEGATE_H

#include <QStyledItemDelegate>
#include "main.h"

class FreqItemDelegate : public QStyledItemDelegate
{
public:
    FreqItemDelegate(QObject * parent) : QStyledItemDelegate(parent)
    {
    }

    QString displayText(const QVariant &value, const QLocale &locale) const
    {
        if (value.userType() == QVariant::Int)
            return locale.toString(khz_to_mhz(value.toInt()), 'f', 1);

        return QStyledItemDelegate::displayText(value, locale);
    }
};


#endif // FREQITEMDELEGATE_H
