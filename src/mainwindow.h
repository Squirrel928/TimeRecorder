#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "TimeRecorder_sol_first.h"
#include <QMainWindow>
#include <qtablewidget.h>
#include <qtimer.h>
#include <vector>

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow, private TimeRecorder_sol_first {
    Q_OBJECT

public:
    MainWindow(QWidget* parent = nullptr);
    ~MainWindow();

private:
    Ui::MainWindow* ui;
    QTimer* m_timer;
    std::vector<std::vector<QTableWidgetItem*>> m_tableItem;

private slots:
    void Timeout();
    void Close_WtiteToSql();
};
#endif // MAINWINDOW_H
