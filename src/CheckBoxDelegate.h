#ifndef CHECKBOXDELEGATE_H
#define CHECKBOXDELEGATE_H

#include <QStyledItemDelegate>

class CheckBoxDelegate : public QStyledItemDelegate
{
    Q_OBJECT
    Q_PROPERTY(QString checkedText READ checkedText WRITE setCheckedText NOTIFY checkedTextChanged)
    Q_PROPERTY(QString uncheckedText READ uncheckedText WRITE setUncheckedText NOTIFY uncheckedTextChanged)
    Q_PROPERTY(QString labelText READ labelText WRITE setLabelText NOTIFY labelTextChanged)
    Q_PROPERTY(bool showLabelAtChecked READ showLabelAtChecked WRITE setShowLabelAtChecked NOTIFY showLabelAtCheckedChanged)
    Q_PROPERTY(bool showLabelAtUnchecked READ showLabelAtUnchecked WRITE setShowLabelAtUnchecked NOTIFY showLabelAtUncheckedChanged)

public:
    explicit CheckBoxDelegate(QObject *parent = nullptr);

    // QAbstractItemDelegate interface
public:
    QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option,
                          const QModelIndex &index) const;
    void setEditorData(QWidget *editor, const QModelIndex &index) const;
    void setModelData(QWidget *editor, QAbstractItemModel *model,
                      const QModelIndex &index) const;
    void updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option,
                              const QModelIndex &index) const;

    // QStyledItemDelegate interface
public:
    QString displayText(const QVariant &value, const QLocale &locale) const;

public:
    const QString &checkedText() const;
    void setCheckedText(const QString &newCheckedText);

    const QString &uncheckedText() const;
    void setUncheckedText(const QString &newUncheckedText);

    const QString &labelText() const;
    void setLabelText(const QString &newLabelText);

    bool showLabelAtChecked() const;
    void setShowLabelAtChecked(bool newShowLabelAtChecked);

    bool showLabelAtUnchecked() const;
    void setShowLabelAtUnchecked(bool newShowLabelAtUnchecked);

signals:
    void checkedTextChanged();
    void uncheckedTextChanged();
    void labelTextChanged();
    void showLabelAtCheckedChanged();
    void showLabelAtUncheckedChanged();

private:
    QString m_checkedText = "✓";
    QString m_uncheckedText; // = "𐄂";
    QString m_labelText = tr("Enabled");
    bool m_showLabelAtChecked = false;
    bool m_showLabelAtUnchecked = false;
};

#endif // CHECKBOXDELEGATE_H
