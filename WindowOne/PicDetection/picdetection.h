#ifndef PICDETECTION_H
#define PICDETECTION_H

#include <QDialog>
#include <QFile>
#include "recognizeimgthread.h"
#include "const.h"

namespace Ui {
class PicDetection;
}

class PicDetection : public QDialog
{
    Q_OBJECT

public:
    explicit PicDetection(QWidget *parent = nullptr);
    ~PicDetection();

private:
    Ui::PicDetection *ui;
    QString _pic_path; // 当前主窗口显示图片的路径
    RecognizeImgThread *_recognize_img_thread = nullptr; // 使用裸指针，交给 Qt 管理
public slots:
    void SlotUpdatePicPath(const QString& _path); // 槽函数：根据传入的路径更新当前图片路径
    void SlotDeletePath(); // 槽函数：当用户触发删除当前图片项的操作时调用
private slots:
    void SlotRecognizeImg();
signals:
    void SigClicked(); // 信号：当用户点击按钮时发出
};

#endif // PICDETECTION_H
