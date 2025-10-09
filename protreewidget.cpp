#include "protreewidget.h"

ProTreeWidget::ProTreeWidget(QWidget *parent):QTreeWidget(parent), _thread_open_pro(nullptr),_open_progressdlg(nullptr)
{

}


void ProTreeWidget::SlotOpenPro(const QString &path)
{
    // 如果路径已经在集合中，说明项目已经打开过，直接返回，不再重复打开
    if(_set_path.find(path) != _set_path.end()){
        return;
    }

    // 将路径插入集合，防止重复打开
    _set_path.insert(path);

    int file_count = 0;               // 用于统计打开项目中的文件数量
    QDir pro_dir(path);               // 创建一个 QDir 对象，用于操作目录
    QString proname = pro_dir.dirName(); // 获取目录名称，即项目名称

    // 创建一个线程对象，用于递归遍历目录，加载项目树
    // std::make_shared 创建一个 shared_ptr，确保线程对象在使用过程中不会被释放
    _thread_open_pro = std::make_shared<OpenTreeThread>(path, file_count, this, nullptr);

    // 创建一个进度对话框，用于显示打开项目的进度
    _open_progressdlg = new QProgressDialog(this);

    // 连接线程的信号与槽函数
    // 当线程更新进度时，调用 SlotUpOpenProgress 更新 UI
    connect(_thread_open_pro.get(), &OpenTreeThread::SigUpdateProgress,
            this, &ProTreeWidget::SlotUpOpenProgress);
    // 当线程完成操作时，调用 SlotFinishOpenProgress
    connect(_thread_open_pro.get(), &OpenTreeThread::SigFinishProgress,
            this, &ProTreeWidget::SlotFinishOpenProgress);

    // 当用户点击进度对话框的取消按钮时，触发槽函数取消线程操作
    connect(_open_progressdlg, &QProgressDialog::canceled,
            this, &ProTreeWidget::SlotCancelOpenProgress);
    // 自定义信号，发射后通知线程停止处理
    connect(this, &ProTreeWidget::SigCancelOpenProgress,
            _thread_open_pro.get(), &OpenTreeThread::SlotCancelProgress);

    // 启动线程，开始处理目录遍历
    _thread_open_pro->start();

    // 配置进度对话框显示属性
    _open_progressdlg->setWindowTitle("Please wait..."); // 设置对话框标题
    _open_progressdlg->setFixedWidth(PROGRESS_WIDTH);    // 固定宽度，防止被拉伸
    _open_progressdlg->setRange(0, PROGRESS_WIDTH);      // 设置进度条范围
    _open_progressdlg->exec();                           // 显示对话框并阻塞当前线程，直到对话框关闭
}



void ProTreeWidget::SlotUpOpenProgress(int count)
{
    if(!_dialog_progress){  // 如果进度条不存在，直接返回
        return;
    }

    // 防止超过最大值，使用取模处理
    if(count >= PROGRESS_MAX){
        _dialog_progress->setValue(count % PROGRESS_MAX);
    } else {
        _dialog_progress->setValue(count);
    }
}


void ProTreeWidget::SlotFinishOpenProgress()
{
    if(!_open_progressdlg){  // 如果进度条不存在，直接返回
        return;
    }
    _open_progressdlg->setValue(PROGRESS_MAX);
    delete _open_progressdlg;
    _open_progressdlg = nullptr;
}


void ProTreeWidget::SlotCancelOpenProgress()
{
    emit SigCancelOpenProgress();                  // 发信号通知线程停止
    delete _open_progressdlg;                   // 删除对话框
    _open_progressdlg = nullptr;                // 防止悬空指针
}





