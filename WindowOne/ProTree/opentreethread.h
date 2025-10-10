#ifndef OPENTREETHREAD_H
#define OPENTREETHREAD_H

#include <QThread>
#include <QTreeWidget>

/**
 * @brief 打开项目树线程类
 * 用于在后台线程中递归遍历目录结构并构建项目树，避免阻塞UI线程
 */
class OpenTreeThread : public QThread
{
    Q_OBJECT

public:
    // 构造函数
    explicit OpenTreeThread(const QString& src_path,QTreeWidget * self, QObject *parent = nullptr);
    // 打开项目树
    void OpenProTree(const QString& src_path, QTreeWidget* self);

protected:
    // 线程执行函数
    virtual void run();

private:
    // 递归构建项目树
    QTreeWidgetItem* RecursiveProTree(
        const QString &src_path,
        QTreeWidget *self,
        QTreeWidgetItem *root,
        QTreeWidgetItem *parent,
        QTreeWidgetItem *preitem);

private:
    QString _src_path;          // 源目录路径
    QTreeWidget* _self;         // 目标树形控件指针
    QTreeWidgetItem* _root;     // 根节点指针
};

#endif // OPENTREETHREAD_H
