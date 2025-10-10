#ifndef PROTREE_H
#define PROTREE_H

#include <QDialog>
#include <QTreeWidget>
#include <QFile>

namespace Ui {
class ProTree;
}

class ProTree : public QDialog
{
    Q_OBJECT

public:
    explicit ProTree(QWidget *parent = nullptr);
    ~ProTree();

private:
    Ui::ProTree *ui;
public:
    QTreeWidget* GetTreeWidget();
};

#endif // PROTREE_H
