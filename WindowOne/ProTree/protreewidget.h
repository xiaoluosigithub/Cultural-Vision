#ifndef PROTREEWIDGET_H
#define PROTREEWIDGET_H

#include <QTreeWidget>
#include <QAction>
#include <QProgressDialog>
#include "opentreethread.h"
#include "removeprodialog.h"

class SlideShowDlg;

/**
 * @brief The ProTreeWidget class
 *        用于管理和显示项目树结构的QTreeWidget扩展控件。
 *        支持导入、设为活动、关闭项目、幻灯片浏览等功能，并与后台线程和进度对话框交互。
 */
class ProTreeWidget : public QTreeWidget
{
    Q_OBJECT
public:
    /**
     * @brief 构造函数
     * @param parent 父控件
     */
    ProTreeWidget(QWidget * parent = nullptr);

    /**
     * @brief 添加一个新的项目树
     * @param name 项目名称
     * @param path 项目路径
     */
    void AddProTree(const QString & name, const QString & path);

private:
    QSet<QString> _set_path;                            ///< 已添加项目路径集合，用于去重
    QTreeWidgetItem * _right_btn_item;                  ///< 右键菜单对应的树项
    QTreeWidgetItem * _selected_item;                   ///< 当前选中的树项
    QAction * _action_closepro;                         ///< 关闭项目动作
    std::shared_ptr<OpenTreeThread> _thread_open_pro;   ///< 项目树打开线程

private slots:
    /**
     * @brief 树项按下处理槽函数 (右键菜单)
     * @param item 被按下的树项
     * @param column 列号
     */
    void SlotItemPressed(QTreeWidgetItem * item, int column);

    /**
     * @brief 树项双击处理槽函数
     * @param doubleItem 被双击的树项
     * @param col 列号
     */
    void SlotDoubleClickItem(QTreeWidgetItem *doubleItem, int col);

    /**
     * @brief 关闭项目槽函数
     */
    void SlotClosePro();

public slots:
    /**
     * @brief 打开项目槽函数
     * @param path 项目路径
     */
    void SlotOpenPro(const QString&  path);

    /**
     * @brief 幻灯片下一张槽函数
     */
    void SlotNextShow();

    /**
     * @brief 幻灯片上一张槽函数
     */
    void SlotPreShow();

signals:

    /**
     * @brief 更新选中项信号
     * @param 选中项路径
     */
    void SigUpdataSelected(const QString& );

    /**
     * @brief 更新图片信号
     * @param 图片路径
     */
    void SigUpdataPic(const QString& );

    /**
     * @brief 清除选中信号
     */
    void SigClearSelected();
};

#endif // PROTREEWIDGET_H
