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

    /**
     * @brief 打开项目树的主函数
     * @param src_path 源目录路径
     * @param self 目标树形控件指针
     */
    void OpenProTree(const QString& src_path, QTreeWidget* self);

protected:
    /**
     * @brief 线程执行函数（重写QThread的虚函数）
     * 线程启动后自动调用此函数
     */
    virtual void run();

private:
    /**
     * @brief 递归构建项目树
     * @param src_path 当前处理的目录路径
     * @param self 目标树形控件指针
     * @param root 根节点指针
     * @param parent 父节点指针
     * @param preitem 前一个兄弟节点指针（用于维护节点间的链接关系）
     */
    QTreeWidgetItem* RecursiveProTree(
        const QString &src_path,
        QTreeWidget *self,
        QTreeWidgetItem *root,
        QTreeWidgetItem *parent,
        QTreeWidgetItem *preitem);

private:
    QString _src_path;          ///< 源目录路径
    QTreeWidget* _self;         ///< 目标树形控件指针
    QTreeWidgetItem* _root;     ///< 根节点指针
};

#endif // OPENTREETHREAD_H
