#include "removeprodialog.h"
#include "ui_removeprodialog.h"

RemoveProDialog::RemoveProDialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::RemoveProDialog)
{
    ui->setupUi(this);
    // 打开样式文件（这里假设 qss 文件放在资源文件中）
    QFile qssFile(":/style/RemoveProDialog.qss");

    // 检查文件是否成功打开（只读方式）
    if (qssFile.open(QFile::ReadOnly)) {

        // 读取整个 QSS 文件内容，并将其转换为 QString
        QString style = QLatin1String(qssFile.readAll());

        // 将样式表应用到当前对话框（this 指向 RemoveProDialog）
        this->setStyleSheet(style);

        // 关闭文件
        qssFile.close();
    }
    this->setWindowTitle("关闭文件");  // 设置窗口标题
    this->setWindowIcon(QIcon(":/icon/close.png"));  // 设置窗口图标（需确保资源路径正确）

}

RemoveProDialog::~RemoveProDialog()
{
    delete ui;
}

bool RemoveProDialog::IsRemoved()
{
    bool bChecked = ui->checkBox->isChecked();
    return bChecked;
}
