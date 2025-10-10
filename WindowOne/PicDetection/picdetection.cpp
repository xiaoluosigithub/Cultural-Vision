#include "picdetection.h"
#include "ui_picdetection.h"

PicDetection::PicDetection(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::PicDetection)
{
    ui->setupUi(this);
    // 假设这是在你的构造函数或者初始化函数中
    ui->tableWidget->setColumnCount(1);  // 设置 1 列
    ui->tableWidget->setRowCount(1);     // 设置 1 行
    ui->tableWidget->setHorizontalHeaderLabels({"图片路径"});  // 可选：设置列标题

}

PicDetection::~PicDetection()
{
    delete ui;
}

void PicDetection::SlotUpdatePicPath(const QString &_path)
{
    _pic_path = _path;
    // 确保表格至少有一行一列（如果没有，可以提前设置，见下文建议）

    // 将路径字符串设置到表格的第 0 行第 0 列
    ui->tableWidget->setItem(0, 0, new QTableWidgetItem(_pic_path));
}

void PicDetection::SlotDeletePath()
{
    _pic_path = "";
    // 将路径字符串设置到表格的第 0 行第 0 列
    ui->tableWidget->setItem(0, 0, new QTableWidgetItem(_pic_path));
}
