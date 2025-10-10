#ifndef PROTREEITEM_H
#define PROTREEITEM_H

#include <QTreeWidgetItem>
#include <QTreeWidget>

/**
 * @brief 自定义树形控件项类，继承自QTreeWidgetItem
 * 用于在树形控件中显示项目，并存储额外的路径信息和导航关系
 */
class ProTreeItem : public QTreeWidgetItem
{
public:
    /**
     * @brief 构造函数 - 作为顶层项
     * @param view 所属的树形控件
     * @param name 项目显示名称
     * @param path 项目对应的文件路径
     * @param type 项目类型，默认为Type
     */
    ProTreeItem(QTreeWidget * view, const QString & name, const QString & path, int type = Type);

    /**
     * @brief 构造函数 - 作为子项
     * @param parent 父项目
     * @param name 项目显示名称
     * @param path 项目对应的文件路径
     * @param root 根项目指针
     * @param type 项目类型，默认为Type
     */
    ProTreeItem(QTreeWidgetItem * parent, const QString & name, const QString & path,
                QTreeWidgetItem * root, int type = Type);

    /**
     * @brief 获取项目对应的文件路径
     * @return 文件路径字符串的常量引用
     */
    const QString & GetPath();

    /**
     * @brief 获取根项目指针
     * @return 根项目的QTreeWidgetItem指针
     */
    QTreeWidgetItem * GetRoot();

    /**
     * @brief 设置前一个同级项目（用于导航）
     * @param item 前一个项目的指针
     */
    void SetPreItem(QTreeWidgetItem * item);

    /**
     * @brief 设置下一个同级项目（用于导航）
     * @param item 下一个项目的指针
     */
    void SetNextItem(QTreeWidgetItem * item);

    /**
     * @brief 获取前一个同级项目
     * @return 前一个项目的ProTreeItem指针
     */
    ProTreeItem * GetPreItem();

    /**
     * @brief 获取下一个同级项目
     * @return 下一个项目的ProTreeItem指针
     */
    ProTreeItem * GetNextItem();
private:
    QString _path;                  ///< 存储项目对应的文件路径
    QString _name;                  ///< 存储项目显示名称
    QTreeWidgetItem * _root;        ///< 指向根项目的指针
    QTreeWidgetItem * _pre_item;    ///< 指向前一个同级项目的指针（用于导航）
    QTreeWidgetItem * _next_item;   ///< 指向下一个同级项目的指针（用于导航）
};

#endif // PROTREEITEM_H
