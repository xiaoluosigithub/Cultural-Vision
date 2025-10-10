#include "protreeitem.h"
#include "const.h"

// 构造函数1：用于创建顶层节点
// 参数 view：树控件 QTreeWidget 的指针
// 参数 name：项目名称
// 参数 path：项目路径
// 参数 type：节点类型（QTreeWidgetItem 的类型，用于区分不同节点）
ProTreeItem::ProTreeItem(QTreeWidget * view, const QString & name,
                         const QString & path, int type)
    // 调用父类构造函数：把该节点挂到 QTreeWidget 上
    :QTreeWidgetItem(view, type),
    _path(path),       // 保存项目路径
    _name(name),       // 保存项目名称
    _root(this),       // 顶层节点的 root 指向自己
    _pre_item(nullptr),// 前一个兄弟节点（初始化为空）
    _next_item(nullptr)// 下一个兄弟节点（初始化为空）
{
}

// 构造函数2：用于创建子节点
// 参数 parent：父节点
// 参数 name：项目名称
// 参数 path：项目路径
// 参数 root：顶层节点的指针（由外部传入）
// 参数 type：节点类型
ProTreeItem::ProTreeItem(QTreeWidgetItem * parent, const QString & name, const QString & path,
                         QTreeWidgetItem * root, int type)
    // 调用父类构造函数：把该节点挂到 parent 节点下
    :QTreeWidgetItem(parent, type),
    _root(root),       // 所属的顶层节点
    _name(name),       // 项目名称
    _path(path),       // 项目路径
    _pre_item(nullptr),// 前一个兄弟节点（初始化为空）
    _next_item(nullptr)// 下一个兄弟节点（初始化为空）
{
}

// 获取路径
const QString &ProTreeItem::GetPath()
{
    return _path;
}

// 获取顶层根节点
QTreeWidgetItem *ProTreeItem::GetRoot()
{
    return _root;
}

// 设置前一个兄弟节点
void ProTreeItem::SetPreItem(QTreeWidgetItem *item)
{
    _pre_item = item;
}

// 设置下一个兄弟节点
void ProTreeItem::SetNextItem(QTreeWidgetItem *item)
{
    _next_item = item;
}

// 获取前一个兄弟节点（转成 ProTreeItem 类型）
// dynamic_cast 用于安全的向下转型，如果不是 ProTreeItem* 会返回 nullptr
ProTreeItem *ProTreeItem::GetPreItem()
{
    return dynamic_cast<ProTreeItem*>(_pre_item);
}

// 获取下一个兄弟节点（转成 ProTreeItem 类型）
ProTreeItem *ProTreeItem::GetNextItem()
{
    return dynamic_cast<ProTreeItem*>(_next_item);
}


