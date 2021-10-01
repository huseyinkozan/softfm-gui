#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>


QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
class QLabel;
QT_END_NAMESPACE


class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    bool isAdvancedMode() const;
    void setIsAdvancedMode(bool newIsAdvancedMode);

private:
    void handleAdvancedModeChange();

private:
    Ui::MainWindow *ui;
    bool m_isAdvancedMode = false;
    QLabel * m_versionLabel = nullptr;
};
#endif // MAINWINDOW_H
