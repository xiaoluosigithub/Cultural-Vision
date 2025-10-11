#ifndef PICSHOW_H
#define PICSHOW_H

#include <QGraphicsOpacityEffect>
#include <QPropertyAnimation>
#include <QDialog>
#include <QString>
#include <QFileInfo>
#include <QFile>

namespace Ui {
class PicShow;
}

// PicShow类：用于展示单张图片的对话框，支持切换上一张/下一张，以及删除等功能
class PicShow : public QDialog
{
    Q_OBJECT

public:
    explicit PicShow(QWidget *parent = nullptr);
    ~PicShow();

protected:
    // 重写event函数，可以在这里处理一些特殊的事件（如键盘、鼠标、绘图事件等）
    bool event(QEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;

private:
    // 控制“上一张”和“下一张”按钮的显示与隐藏
    void ShowPreNextBtns(bool b_show);

    QString GetSelectedPath();

    Ui::PicShow *ui;

    // 动画对象：用于控制“上一张”按钮的显示/隐藏动画效果（如淡入淡出）
    QPropertyAnimation* _animation_show_pre;

    // 动画对象：用于控制“下一张”按钮的显示/隐藏动画效果
    QPropertyAnimation* _animation_show_next;

    // 标志位：表示“上一张”和“下一张”按钮当前是否可见
    bool _b_btnvisible;

    // 当前选中并展示的图片的完整路径
    QString _selected_path;

    // 当前加载的图片，使用QPixmap存储图像数据，便于显示
    QPixmap _pix_map;

public slots:
    // 槽函数：当用户选择了一个新的图片项时调用，传入该图片的路径
    void SlotSelectItem(const QString& path);

    // 槽函数：根据传入的路径更新当前显示的图片内容
    void SlotUpdatePic(const QString& _path);

    // 槽函数：当用户触发删除当前图片项的操作时调用
    void SlotDeleteItem();

signals:
    // 信号：当用户点击“下一张”按钮时发出，通知外部切换到下一张图片
    void SigNextClicked();

    // 信号：当用户点击“上一张”按钮时发出，通知外部切换到上一张图片
    void SigPreClicked();
};

#endif // PICSHOW_H
