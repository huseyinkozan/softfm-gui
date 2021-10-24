#include "CheckBoxDelegate.h"
#include <QCheckBox>

CheckBoxDelegate::CheckBoxDelegate(QObject *parent)
    : QStyledItemDelegate(parent)
{

}

QWidget *CheckBoxDelegate::createEditor(QWidget *parent,
                                        const QStyleOptionViewItem &option,
                                        const QModelIndex &index) const
{
    Q_UNUSED(option);
    Q_UNUSED(index);

    QCheckBox * cb = new QCheckBox(parent);
    cb->setText(m_labelText);

    return cb;
}

void CheckBoxDelegate::setEditorData(QWidget *editor,
                                     const QModelIndex &index) const
{
    const bool val = index.model()->data(index, Qt::EditRole).toBool();
    QCheckBox * cb = qobject_cast<QCheckBox*>(editor);
    if ( ! cb)
        return;
    cb->setChecked(val);
}

void CheckBoxDelegate::setModelData(QWidget *editor, QAbstractItemModel *model,
                                    const QModelIndex &index) const
{
    QCheckBox * cb = qobject_cast<QCheckBox*>(editor);
    if ( ! cb)
        return;
    const bool val = cb->isChecked();
    model->setData(index, val, Qt::EditRole);
}

void CheckBoxDelegate::updateEditorGeometry(QWidget *editor,
                                            const QStyleOptionViewItem &option,
                                            const QModelIndex &index) const
{
    Q_UNUSED(index);
    editor->setGeometry(option.rect);
}

QString CheckBoxDelegate::displayText(const QVariant &value,
                                      const QLocale &locale) const
{
    Q_UNUSED(locale);

    const bool val = value.toBool();
    if (val) {
        if (m_showLabelAtChecked)
            return QString("%1 %2").arg(m_checkedText, m_labelText);
        else
            return m_checkedText;
    } else {
        if (m_showLabelAtUnchecked)
            return QString("%1 %2").arg(m_uncheckedText, m_labelText);
        else
            return m_uncheckedText;
    }
}

const QString &CheckBoxDelegate::checkedText() const
{
    return m_checkedText;
}

void CheckBoxDelegate::setCheckedText(const QString &newCheckedText)
{
    if (m_checkedText == newCheckedText)
        return;
    m_checkedText = newCheckedText;
    emit checkedTextChanged();
}

const QString &CheckBoxDelegate::uncheckedText() const
{
    return m_uncheckedText;
}

void CheckBoxDelegate::setUncheckedText(const QString &newUncheckedText)
{
    if (m_uncheckedText == newUncheckedText)
        return;
    m_uncheckedText = newUncheckedText;
    emit uncheckedTextChanged();
}

const QString &CheckBoxDelegate::labelText() const
{
    return m_labelText;
}

void CheckBoxDelegate::setLabelText(const QString &newLabelText)
{
    if (m_labelText == newLabelText)
        return;
    m_labelText = newLabelText;
    emit labelTextChanged();
}

bool CheckBoxDelegate::showLabelAtChecked() const
{
    return m_showLabelAtChecked;
}

void CheckBoxDelegate::setShowLabelAtChecked(bool newShowLabelAtChecked)
{
    if (m_showLabelAtChecked == newShowLabelAtChecked)
        return;
    m_showLabelAtChecked = newShowLabelAtChecked;
    emit showLabelAtCheckedChanged();
}

bool CheckBoxDelegate::showLabelAtUnchecked() const
{
    return m_showLabelAtUnchecked;
}

void CheckBoxDelegate::setShowLabelAtUnchecked(bool newShowLabelAtUnchecked)
{
    if (m_showLabelAtUnchecked == newShowLabelAtUnchecked)
        return;
    m_showLabelAtUnchecked = newShowLabelAtUnchecked;
    emit showLabelAtUncheckedChanged();
}
