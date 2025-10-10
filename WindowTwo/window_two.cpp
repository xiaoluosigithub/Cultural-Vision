#include "window_two.h"
#include "ui_window_two.h"

window_two::window_two(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::window_two)
{
    ui->setupUi(this);
    setWindowTitle("窗口2");
    connect(ui->closeBtn, &QPushButton::clicked, this, &window_two.close());
    connect(this, &QWidget::destroyed, this, &window_two_Closed(this));
}

window_two::~window_two()
{
    delete ui;
}
