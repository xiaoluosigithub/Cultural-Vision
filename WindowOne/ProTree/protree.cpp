#include "protree.h"
#include "ui_protree.h"

ProTree::ProTree(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::ProTree)
{
    ui->setupUi(this);
    // 打开样式文件（这里假设 qss 文件放在资源文件中）
    QFile qssFile(":/style/ProTree.qss");

    // 检查文件是否成功打开（只读方式）
    if (qssFile.open(QFile::ReadOnly)) {

        // 读取整个 QSS 文件内容，并将其转换为 QString
        QString style = QLatin1String(qssFile.readAll());

        // 将样式表应用到当前对话框（this 指向 RemoveProDialog）
        this->setStyleSheet(style);

        // 关闭文件
        qssFile.close();
    }
    ui->treeWidget->setRootIsDecorated(true);
    ui->treeWidget->setItemsExpandable(true);
    ui->treeWidget->setAnimated(true);

    QTreeWidgetItem *root = ui->treeWidget->topLevelItem(0);
    if (root) root->setChildIndicatorPolicy(QTreeWidgetItem::ShowIndicator);
}

ProTree::~ProTree()
{
    delete ui;
}


QTreeWidget *ProTree::GetTreeWidget()
{
    return ui->treeWidget;
}

