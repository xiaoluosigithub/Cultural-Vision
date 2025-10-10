#include "opentreethread.h"
#include <QDir>
#include "protreeitem.h"
#include "const.h"

OpenTreeThread::OpenTreeThread(const QString &src_path, QTreeWidget *self, QObject *parent)
    :QThread(parent), _src_path(src_path), _self(self), _root(nullptr)
{

}

void OpenTreeThread::OpenProTree(
    const QString &src_path,   // 项目根目录路径
    QTreeWidget *self          // 树控件指针，用于添加节点
    )
{
    QDir src_dir(src_path);               // 创建 QDir 对象，用于操作目录
    auto name = src_dir.dirName();        // 获取目录名称，作为项目名

    // 创建一个表示项目根节点的 ProTreeItem
    auto * item = new ProTreeItem(self, name, src_path, TreeItemPro);
    item->setData(0, Qt::DisplayRole, name);              // 设置显示名称
    item->setData(0, Qt::DecorationRole, QIcon(":/icon/dir.png")); // 设置图标为文件夹图标
    item->setData(0, Qt::ToolTipRole, src_path);         // 设置鼠标悬停提示为完整路径

    _root = item;  // 保存根节点指针，供递归使用

    // 调用递归函数，遍历项目目录，构建树结构
    RecursiveProTree(src_path, self, _root, item, nullptr);
}


void OpenTreeThread::run()
{
    OpenProTree(_src_path, _self);
}


QTreeWidgetItem* OpenTreeThread::RecursiveProTree(
    const QString &src_path,
    QTreeWidget *self,
    QTreeWidgetItem *root,
    QTreeWidgetItem *parent,
    QTreeWidgetItem *preitem // 输入为上层的最后节点
    )
{
    QDir src_dir(src_path);
    src_dir.setFilter(QDir::Dirs | QDir::Files | QDir::NoDotAndDotDot);
    src_dir.setSorting(QDir::Name);
    QFileInfoList list = src_dir.entryInfoList();

    QTreeWidgetItem *lastItem = preitem; // 当前层的最后节点

    for (int i = 0; i < list.size(); ++i) {
        QFileInfo fileInfo = list.at(i);
        bool bIsDir = fileInfo.isDir();

        if (bIsDir) {
            // 如果是文件夹，不创建节点，只递归进去
            lastItem = RecursiveProTree(fileInfo.absoluteFilePath(), self, root, parent, lastItem);
        } else {
            // 仅对文件创建节点
            ProTreeItem *item = new ProTreeItem(parent,
                                                fileInfo.fileName(),
                                                fileInfo.absoluteFilePath(),
                                                _root,
                                                TreeItemPic);

            item->setData(0, Qt::DisplayRole, fileInfo.fileName());
            item->setData(0, Qt::DecorationRole, QIcon(":/icon/pic.png"));
            item->setData(0, Qt::ToolTipRole, fileInfo.absoluteFilePath());

            // 维护链表
            if (lastItem) {
                auto *pre_proitem = dynamic_cast<ProTreeItem*>(lastItem);
                pre_proitem->SetNextItem(item);
                item->SetPreItem(lastItem);
            }
            lastItem = item;
        }
    }

    return lastItem; // 返回当前层最后的节点
}

