#include "picshow.h"
#include "ui_picshow.h"


PicShow::PicShow(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::PicShow)
{
    ui->setupUi(this);
    ui->previousBtn->SetIcons(":/icon/previous.png",
                              ":/icon/previous_hover.png",
                              ":/icon/previous_press.png");
    ui->nextBtn->SetIcons(":/icon/next.png",
                          ":/icon/next_hover.png",
                          ":/icon/next_press.png");
    QGraphicsOpacityEffect * opacity_pre = new QGraphicsOpacityEffect(this);
    opacity_pre->setOpacity(0);
    ui->previousBtn->setGraphicsEffect(opacity_pre);

    QGraphicsOpacityEffect * opacity_next = new QGraphicsOpacityEffect(this);
    opacity_next->setOpacity(0);
    ui->nextBtn->setGraphicsEffect(opacity_next);

    _animation_show_pre = new QPropertyAnimation(opacity_pre, "opacity", this);
    _animation_show_pre->setEasingCurve(QEasingCurve::Linear);
    _animation_show_pre->setDuration(500);

    _animation_show_next = new QPropertyAnimation(opacity_next,"opacity", this);
    _animation_show_next->setEasingCurve(QEasingCurve::Linear);
    _animation_show_next->setDuration(500);

    connect(ui->nextBtn, &QPushButton::clicked, this, &PicShow::SigNextClicked);
    connect(ui->previousBtn, &QPushButton::clicked, this, &PicShow::SigPreClicked);

}

PicShow::~PicShow()
{
    delete ui;
}

// 返回主页面展示图片路径
QString PicShow::GetSelectedPath()
{
    return _selected_path;
}

bool PicShow::event(QEvent *event)
{
    switch(event->type()){
    case QEvent::Enter:
        ShowPreNextBtns(true);   // 当鼠标进入窗口时，显示“上一张”和“下一张”按钮
        break;
    case QEvent::Leave:
        ShowPreNextBtns(false);  // 当鼠标离开窗口时，隐藏“上一张”和“下一张”按钮
        break;
    default:
        break;
    }

    return QDialog::event(event);
}

void PicShow::ShowPreNextBtns(bool b_show)
{
    if(!b_show && _b_btnvisible){
        _animation_show_pre->stop();
        _animation_show_pre->setStartValue(1);
        _animation_show_pre->setEndValue(0);
        _animation_show_pre->start();

        _animation_show_next->stop();
        _animation_show_next->setStartValue(1);
        _animation_show_next->setEndValue(0);
        _animation_show_next->start();

        _b_btnvisible = false;

        return;
    }

    if(b_show && !_b_btnvisible){
        _animation_show_pre->stop();
        _animation_show_pre->setStartValue(0);
        _animation_show_pre->setEndValue(1);
        _animation_show_pre->start();

        _animation_show_next->stop();
        _animation_show_next->setStartValue(0);
        _animation_show_next->setEndValue(1);
        _animation_show_next->start();

        _b_btnvisible = true;

        return;
    }

}

void PicShow::SlotSelectItem(const QString &path)
{
    _selected_path = path;
    _pix_map.load(path);
    auto width = this->width() - 20;
    auto height = this->height() - 20;
    _pix_map = _pix_map.scaled(width, height, Qt::KeepAspectRatio);
    ui->label->setPixmap(_pix_map);
}

void PicShow::SlotUpdatePic(const QString &_path)
{
    _selected_path = _path;

    if (_selected_path.isEmpty()) {
        ui->label->clear();
        return;
    }

    QFileInfo fileInfo(_selected_path);

    // 判断路径类型
    if (fileInfo.isDir()) {
        // 是文件夹，显示提示或图标
        // QPixmap folderIcon(":/icon/dir.png"); // 可以换成合适的资源路径
        // folderIcon = folderIcon.scaled(width, height, Qt::KeepAspectRatio, Qt::SmoothTransformation);
        // ui->label->setPixmap(folderIcon);

        // 显示文字提示
        ui->label->setText("📁 当前路径是文件夹");

        return;
    }

    // 文件（图片）情况
    if (fileInfo.isFile()) {
        const auto width = ui->gridLayout->geometry().width();
        const auto height = ui->gridLayout->geometry().height();

        if (_pix_map.load(_selected_path)) {
            _pix_map = _pix_map.scaled(width, height, Qt::KeepAspectRatio, Qt::SmoothTransformation);
            ui->label->setPixmap(_pix_map);
        } else {
            ui->label->setText("⚠️ 无法加载图片");
        }
    }
}

void PicShow::SlotDeleteItem()
{
    _selected_path = "";
}
