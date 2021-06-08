#include "mainwindow.h"

#include <QApplication>

int main(int argc, char* argv[]) {
    QApplication a(argc, argv);
    MainWindow w;
    a.connect(&a, SIGNAL(lastWindowClosed()), &w, SLOT(Close_WtiteToSql()));
    w.show();
    return a.exec();
}
