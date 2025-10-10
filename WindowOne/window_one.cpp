#include "window_one.h"
#include "ui_window_one.h"

window_one::window_one(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::window_one)
{
    ui->setupUi(this);
    setWindowTitle("窗口1");
    connect(ui->closeBtn, &QPushButton::clicked, this, &window_one.close());
    connect(this, &QWidget::destroyed, this, &window_one_Closed(this));
}

window_one::~window_one()
{
    delete ui;
}
