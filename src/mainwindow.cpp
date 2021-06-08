#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <qdebug.h>
#include <qguiapplication.h>
#include <qrect.h>
#include <qscreen.h>

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent), ui(new Ui::MainWindow) {
    ui->setupUi(this);

    setWindowTitle(tr("TimeRecorder"));

    QRect mRect = QGuiApplication::primaryScreen()->geometry(); //获取屏幕分辨率
    move(mRect.width() - width() - 150, 150);                   //窗口位置

    // m_timer
    m_timer = new QTimer(this);
    connect(m_timer, &QTimer::timeout, this, &MainWindow::Timeout);
    m_timer->start(1000);

    // ui->tableWidget
    ui->tableWidget->setColumnWidth(0, 150);

    // m_tableItem
    m_tableItem = std::vector<std::vector<QTableWidgetItem*>>(
        ui->tableWidget->rowCount(),
        std::vector<QTableWidgetItem*>(ui->tableWidget->columnCount(),
                                       nullptr));
    for (int i = 0; i < ui->tableWidget->rowCount(); i++)
        for (int j = 0; j < ui->tableWidget->columnCount(); j++) {
            m_tableItem[i][j] = new QTableWidgetItem;
            ui->tableWidget->setItem(i, j, m_tableItem[i][j]);
        }
}

MainWindow::~MainWindow() {
    delete ui;
    for (int i = 0; i < ui->tableWidget->rowCount(); i++)
        for (int j = 0; j < ui->tableWidget->columnCount(); j++) {
            delete m_tableItem[i][j];
        }
}

void MainWindow::Timeout() {
    this->Update();
    this->Sort();
    int i = 0;
    for (; i < ui->tableWidget->rowCount(); i++) {
        TimeRecorder::_appData* t = &this->m_runPro[i];
        if (!t->running)
            break;
        m_tableItem[i][0]->setText(QString::fromStdWString(
            t->inSql ? t->sqlData.name : t->sqlData.exeName));
        m_tableItem[i][1]->setText(
            QString::fromStdWString(TimeRecorder::TimeToString(RunTime(*t))));
        m_tableItem[i][2]->setText(QString::fromStdWString(
            t->inSql
                ? TimeRecorder::TimeToString(RunTime(*t) + t->sqlData.totalTime)
                : TimeRecorder::TimeToString(RunTime(*t))));
    }
    if (i < ui->tableWidget->rowCount()) {
        m_tableItem[i][0]->setText("");
        m_tableItem[i][1]->setText("");
        m_tableItem[i][2]->setText("");
        i++;
    }
    if (i < ui->tableWidget->rowCount()) {
        m_tableItem[i][0]->setText(tr("名称"));
        m_tableItem[i][1]->setText(tr("最后运行"));
        m_tableItem[i][2]->setText(tr("总计时间"));
        i++;
    }
    for (; i < ui->tableWidget->rowCount(); i++) {
        if (i - 2 >= m_runPro.size())
            break;
        TimeRecorder::_appData* t = &this->m_runPro[i - 2];
        m_tableItem[i][0]->setText(QString::fromStdWString(t->sqlData.exeName));
        m_tableItem[i][1]->setText("");
        m_tableItem[i][2]->setText(QString::fromStdWString(
            TimeRecorder::TimeToString(t->sqlData.totalTime)));
    }
}

void MainWindow::Close_WtiteToSql() {
    qDebug() << "hello\n";
    for (auto& i : m_runPro)
        if (i.inSql && i.running) {
            time_t nowTime = GetTime();
            i.total += nowTime - i.front;
            m_sql->addRecord(i.sqlData.sqlId, i.begin, nowTime, i.total);
        }
    m_runPro.clear();
    TimeRecorder::log(0) << "Endding time: "
                         << TimeRecorder::TimeToString(time(NULL), true);
}