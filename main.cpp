#include "mainwindow.h"

#include <QApplication>
#include <QFile>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow w;
    // 设置窗口标题
    w.setWindowTitle("Cultural-Vision");
    // 显示窗口
    w.show();
    return a.exec();
}
