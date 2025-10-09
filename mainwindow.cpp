// mainwindow.cpp
#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

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
