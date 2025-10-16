// window_two.h
#ifndef WINDOW_TWO_H
#define WINDOW_TWO_H

#include "camerathread.h"
#include "settingdialog.h"
#include "recognizeimgthread.h"
#include "mainwindow.h"
#include "const.h"
#include <QDialog>
#include <QImage>
#include <QLabel>
#include <QCloseEvent>
#include <QFile>

namespace Ui { class WindowTwo; }

class WindowTwo : public QDialog
{
    Q_OBJECT

public:
    explicit WindowTwo(QWidget *parent = nullptr);
    ~WindowTwo();

signals:
    void windowClosed();

private slots:
    void handleClose();
    void on_btnStart_clicked(); // 当用户点击"开始"按钮时调用，用于启动相机捕获线程。
    void on_btnStop_clicked(); // 当用户点击"停止"按钮时调用，用于停止相机捕获线程。
    void on_setBtn_clicked();   // 当用户点击"设置"按钮时打开设置界面，用于设置相机。
    void updateFrame(const QImage &image); //  当相机线程捕获到新帧并发出frameReady信号时调用，用于在UI中更新显示的图像。
    // 新增槽函数：接收识别结果
    void onRecognizeSuccess(QString className, float confidence);
    void onRecognizeFail(QString errorMsg);
protected:
    void closeEvent(QCloseEvent *event) override;

private:
    Ui::WindowTwo *ui;

    int selectedCamera = 0;         // 当前摄像头索引
    RecognizeImgThread *recognizeThread = nullptr; // 新增：识别线程
    CameraThread *cameraThread;  // 指向相机线程对象的指针，用于管理相机捕获

    QString labelPath;
    QString modelPath;
};

#endif // WINDOW_TWO_H
