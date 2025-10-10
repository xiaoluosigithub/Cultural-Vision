// window_one.cpp
#include "windowone.h"
#include "ui_windowone.h"
#include <QCloseEvent>

WindowOne::WindowOne(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::WindowOne)
{
    ui->setupUi(this);
    setWindowTitle("窗口1");

    // 关闭按钮
    connect(ui->closeBtn, &QPushButton::clicked, this, &WindowOne::handleClose);

    // 打开项目
    connect(ui->openBtn, &QPushButton::clicked, this, &WindowOne::SlotOpenPro);

    // 创建项目树
    _protree = new ProTree();
    // 将项目树加载到页面上
    ui->proLayout->addWidget(_protree);

    // 拿到项目树页面中的项目树窗口
    QTreeWidget* tree_widget = dynamic_cast<ProTree*>(_protree)->GetTreeWidget();
    auto * pro_tree_widget = dynamic_cast<ProTreeWidget*>(tree_widget);

    connect(this, &WindowOne::SigOpenPro, pro_tree_widget, &ProTreeWidget::SlotOpenPro);

    // 创建图片页面
    _picshow = new PicShow();

    // 将图片页面展示到pic页面上
    ui->picLayout->addWidget(_picshow);

    // 基类转换子类
    auto *pro_pic_show = dynamic_cast<PicShow*>(_picshow);

    // 连接图片页面和项目树控件的信号与槽

    // 项目树控件选中项变化，通知图片页面进行展示
    connect(pro_tree_widget, &ProTreeWidget::SigUpdataSelected, pro_pic_show, &PicShow::SlotSelectItem);

    // 图片页面点击“下一张”按钮，通知项目树控件切换到下一个项目
    connect(pro_pic_show, &PicShow::SigNextClicked, pro_tree_widget, &ProTreeWidget::SlotNextShow);

    // 图片页面点击“上一张”按钮，通知项目树控件切换到上一个项目
    connect(pro_pic_show, &PicShow::SigPreClicked, pro_tree_widget, &ProTreeWidget::SlotPreShow);

    // 项目树控件中的图片更新时，通知图片页面刷新图片展示
    connect(pro_tree_widget, &ProTreeWidget::SigUpdataPic, pro_pic_show, &PicShow::SlotUpdatePic);

    // 项目树控件需要清除选中项时，通知图片页面删除对应图片
    connect(pro_tree_widget, &ProTreeWidget::SigClearSelected, pro_pic_show, &PicShow::SlotDeleteItem);



    _picdete = new PicDetection();
    ui->resLayout->addWidget(_picdete);
    auto * pro_res_show = dynamic_cast<PicDetection*>(_picdete);

    // 项目树控件选中项变化，更新图片路径
    connect(pro_tree_widget, &ProTreeWidget::SigUpdataSelected, pro_res_show, &PicDetection::SlotUpdatePicPath);

    // 项目树控件中的图片更新，更新图片路径
    connect(pro_tree_widget, &ProTreeWidget::SigUpdataPic, pro_res_show, &PicDetection::SlotUpdatePicPath);

    // 清楚选中项时 图片路径置空
    connect(pro_tree_widget, &ProTreeWidget::SigClearSelected, pro_res_show, &PicDetection::SlotDeletePath);



}

WindowOne::~WindowOne()
{
    delete ui;
}

void WindowOne::handleClose()
{
    emit windowClosed();
    this->close();
}

void WindowOne::SlotOpenPro(bool)
{
    // 创建一个文件对话框对象
    QFileDialog file_dialog;
    // 设置文件模式为选择目录（而不是单个文件）
    file_dialog.setFileMode(QFileDialog::Directory);
    // 设置对话框标题
    file_dialog.setWindowTitle(tr("选择导入的文件夹"));
    // 设置对话框默认打开的路径为当前程序的运行目录
    file_dialog.setDirectory(QDir::currentPath());
    // 设置文件视图模式为详细信息（列表+属性）
    file_dialog.setViewMode(QFileDialog::Detail);

    QStringList fileNames;
    // 打开文件对话框，如果用户点击了“确定”按钮（exec()返回非0值）
    if(file_dialog.exec()){
        // 获取用户选择的目录（可能是多个，但这里只取第一个）
        fileNames = file_dialog.selectedFiles();
    }

    // 如果用户没有选择任何目录，则直接返回
    if(fileNames.length() <= 0){
        return;
    }

    // 获取第一个用户选择的路径
    QString import_path = fileNames.at(0);
    // 发出信号，将选择的目录路径传递出去
    emit SigOpenPro(import_path);
}

void WindowOne::closeEvent(QCloseEvent *event)
{
    emit windowClosed();
    QDialog::closeEvent(event);
}
