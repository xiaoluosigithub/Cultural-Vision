#include "mainwindow.h"

#include <QApplication>
#include <QFile>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    // 创建QFile对象读取QSS样式表文件（使用Qt资源系统）
    QFile qss(":/style/style.qss");
    // 尝试以只读方式打开QSS文件
    if(qss.open(QFile::ReadOnly)){
        qDebug() << "open qss success" << Qt::endl;
        // 读取全部内容并转换为QString
        QString style = QLatin1String(qss.readAll());
        // 将样式表应用到整个应用程序
        a.setStyleSheet(style);
        // 关闭文件（良好习惯，但QFile析构时会自动关闭）
        qss.close();
    } else {
        qDebug() << "open qss filed" << Qt::endl;
        return 0;
    }
    MainWindow w;
    // 设置窗口标题
    w.setWindowTitle("Cultural-Vision");
    // 显示窗口
    w.show();
    return a.exec();
}
