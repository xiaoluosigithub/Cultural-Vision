#include "protree.h"
#include "ui_protree.h"

ProTree::ProTree(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::ProTree)
{
    ui->setupUi(this);
}

ProTree::~ProTree()
{
    delete ui;
}


QTreeWidget *ProTree::GetTreeWidget()
{
    return ui->treeWidget;
}

