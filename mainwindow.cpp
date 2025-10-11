// mainwindow.cpp
#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    // 打开样式文件（这里假设 qss 文件放在资源文件中）
    QFile qssFile(":/style/MainWindow.qss");

    // 检查文件是否成功打开（只读方式）
    if (qssFile.open(QFile::ReadOnly)) {

        // 读取整个 QSS 文件内容，并将其转换为 QString
        QString style = QLatin1String(qssFile.readAll());

        // 将样式表应用到当前对话框（this 指向 RemoveProDialog）
        this->setStyleSheet(style);

        // 关闭文件
        qssFile.close();
    }


    // 连接主界面按钮
    connect(ui->pushButton1, &QPushButton::clicked, this, &MainWindow::openWindowOne);
    connect(ui->pushButton2, &QPushButton::clicked, this, &MainWindow::openWindowTwo);

    // 连接副界面关闭信号
    connect(&windowOne, &WindowOne::windowClosed, this, &MainWindow::windowOneClosed);
    connect(&windowTwo, &WindowTwo::windowClosed, this, &MainWindow::windowTwoClosed);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::disableMainButtons()
{
    ui->pushButton1->setEnabled(false);
    ui->pushButton2->setEnabled(false);
}

void MainWindow::enableMainButtons()
{
    ui->pushButton1->setEnabled(true);
    ui->pushButton2->setEnabled(true);
}

void MainWindow::openWindowOne()
{
    disableMainButtons();
    if (!windowOne.isVisible()) {
        windowOne.show();
    } else {
        windowOne.raise();
        windowOne.activateWindow();
    }
}

void MainWindow::openWindowTwo()
{
    disableMainButtons();
    if (!windowTwo.isVisible()) {
        windowTwo.show();
    } else {
        windowTwo.raise();
        windowTwo.activateWindow();
    }
}

void MainWindow::windowOneClosed()
{
    enableMainButtons();
}

void MainWindow::windowTwoClosed()
{
    enableMainButtons();
}
