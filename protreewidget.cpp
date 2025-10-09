#include "protreewidget.h"
#include <QDir>
#include "protreeitem.h"
#include "const.h"
#include <QGuiApplication>
#include <QMenu>
#include <QFileDialog>


ProTreeWidget::ProTreeWidget(QWidget *parent):QTreeWidget(parent),
    _right_btn_item(nullptr), _selected_item(nullptr),
    _thread_open_pro(nullptr)

{
    // 隐藏树控件的表头（不显示列标题），更像一个文件浏览树
    this->setHeaderHidden(true);

    // 连接信号槽：当用户点击树节点时，触发 SlotItemPressed 函数
    connect(this, &ProTreeWidget::itemPressed, this, &ProTreeWidget::SlotItemPressed);

    // 创建右键菜单的动作（Action）

    _action_closepro = new QAction(QIcon(":/icon/close.png"), tr("关闭项目"), this);
    // 图标：close.png，显示文本：关闭项目

    // 连接动作触发信号与槽函数
    connect(_action_closepro, &QAction::triggered, this, &ProTreeWidget::SlotClosePro);

    connect(this, &ProTreeWidget::itemDoubleClicked, this, &ProTreeWidget::SlotDoubleClickItem);
}

void ProTreeWidget::AddProTree(const QString &name, const QString &path)
{
    // 生成项目的完整路径 = path + / + name
    QDir dir(path);
    QString file_path = dir.absoluteFilePath(name);
    // 如果这个路径已经存在于集合中， 直接返回（防止重复添加）
    if(_set_path.find(file_path) != _set_path.end()){
        return;
    }
    // 检查项目路径是否存在，不存在就创建
    QDir pro_dir(file_path);
    if(!pro_dir.exists()){
        // 递归创建目录
        bool enable = pro_dir.mkpath(file_path);
        if(!enable){
            return;
        }
    }

    // 将路径加入集合， 避免后续重复添加
    _set_path.insert(file_path);
    // 创建一个自定义的树节点
    auto * item = new ProTreeItem(this, name, file_path, TreeItemPro);
    // 设置节点显示的文字
    item->setData(0, Qt::DisplayRole, name);
    // 设置节点显示的图标
    item->setData(0, Qt::DecorationRole, QIcon(":/icon/dir.png"));
    // 设置鼠标悬停时的提示信息
    item->setData(0, Qt::ToolTipRole, file_path);
    // 将新节点添加为顶层节点
    this->addTopLevelItem(item);
}

// 当用户点击树节点时触发
void ProTreeWidget::SlotItemPressed(QTreeWidgetItem *pressedItem, int column)
{
    // 判断是否为鼠标右键点击
    if(QGuiApplication::mouseButtons() == Qt::RightButton){
        QMenu menu(this);  // 创建右键菜单
        int itemtype = pressedItem->type();  // 获取节点类型
        if(itemtype == TreeItemPro){  // 如果是项目类型节点
            _right_btn_item = pressedItem; // 记录当前右键点击的节点，用于后续操作
            // 添加菜单操作项
            menu.addAction(_action_closepro);   // 关闭项目
            menu.exec(QCursor::pos());          // 在鼠标当前位置显示菜单
        }
    }
}

void ProTreeWidget::SlotDoubleClickItem(QTreeWidgetItem *doubleItem, int col)
{
    if(QGuiApplication::mouseButtons() == Qt::LeftButton){
        auto * tree_doubleItem = dynamic_cast<ProTreeItem*>(doubleItem);
        if(!tree_doubleItem){
            return;
        }
        int itemtype = tree_doubleItem->type();
        if(itemtype == TreeItemPic){
            emit SigUpdataSelected(tree_doubleItem->GetPath());
            _selected_item = doubleItem;
        }
    }
}

// 关闭项目
void ProTreeWidget::SlotClosePro()
{
    // 创建一个“删除项目”对话框
    RemoveProDialog remove_pro_dialog;

    // 显示对话框并阻塞等待用户操作
    auto res = remove_pro_dialog.exec();

    // 如果用户没有点击“确定”（即不是 Accepted），直接返回，不执行删除操作
    if(res != QDialog::Accepted){
        return;
    }

    // 判断用户是否选择同时删除硬盘上的项目文件
    bool b_remove = remove_pro_dialog.IsRemoved();

    // 获取当前右键选中的顶层节点在树控件中的索引
    auto index_right_btn = this->indexOfTopLevelItem(_right_btn_item);

    // 将右键选中的节点转换为 ProTreeItem 类型，以便访问项目路径等自定义方法
    auto * protreeitem = dynamic_cast<ProTreeItem*>(_right_btn_item);

    auto * selecteditem = dynamic_cast<ProTreeItem*>(_selected_item);

    // 获取项目路径
    auto delete_path = protreeitem->GetPath();

    // 从已打开项目集合中移除该路径，防止重复打开
    _set_path.remove(delete_path);

    // 如果用户选择删除硬盘文件，则递归删除整个目录
    if(b_remove){
        QDir delete_dir(delete_path);
        delete_dir.removeRecursively();
    }


    if(selecteditem && protreeitem == selecteditem->GetRoot()){
        selecteditem = nullptr;
        _selected_item = nullptr;
        emit SigClearSelected();
    }

    // 从树控件中移除顶层节点，并释放节点内存
    delete this->takeTopLevelItem(index_right_btn);

    // 清空右键选中节点指针，防止悬空指针
    _right_btn_item = nullptr;
}


// 打开项目
void ProTreeWidget::SlotOpenPro(const QString &path)
{
    // 如果路径已经在集合中，说明项目已经打开过，直接返回，不再重复打开
    if(_set_path.find(path) != _set_path.end()){
        return;
    }

    // 将路径插入集合，防止重复打开
    _set_path.insert(path);

    QDir pro_dir(path);               // 创建一个 QDir 对象，用于操作目录
    QString proname = pro_dir.dirName(); // 获取目录名称，即项目名称

    // 创建一个线程对象，用于递归遍历目录，加载项目树
    // std::make_shared 创建一个 shared_ptr，确保线程对象在使用过程中不会被释放
    _thread_open_pro = std::make_shared<OpenTreeThread>(path, this, nullptr);

    // 启动线程，开始处理目录遍历
    _thread_open_pro->start();
}

void ProTreeWidget::SlotNextShow()
{
    if(!_selected_item){
        return;
    }
    auto *curItem = dynamic_cast<ProTreeItem*>(_selected_item)->GetNextItem();
    if(!curItem){
        return;
    }
    emit SigUpdataPic(curItem->GetPath());
    _selected_item = curItem;
    this->setCurrentItem(curItem);
}

void ProTreeWidget::SlotPreShow()
{
    if(!_selected_item){
        return;
    }
    auto *curItem = dynamic_cast<ProTreeItem*>(_selected_item)->GetPreItem();
    if(!curItem){
        return;
    }
    emit SigUpdataPic(curItem->GetPath());
    _selected_item = curItem;
    this->setCurrentItem(curItem);
}


