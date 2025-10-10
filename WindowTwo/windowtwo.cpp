// window_two.cpp
#include "windowtwo.h"
#include "ui_windowtwo.h"
#include <QCloseEvent>

WindowTwo::WindowTwo(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::WindowTwo)
{
    ui->setupUi(this);
    setWindowTitle("窗口2");

    // 关闭按钮
    connect(ui->closeBtn, &QPushButton::clicked, this, &WindowTwo::handleClose);

}

WindowTwo::~WindowTwo()
{
    delete ui;
}

void WindowTwo::handleClose()
{
    emit windowClosed();
    this->close();
}

void WindowTwo::closeEvent(QCloseEvent *event)
{
    emit windowClosed();
    QDialog::closeEvent(event);
}
