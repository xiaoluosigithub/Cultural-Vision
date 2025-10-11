// window_two.cpp
#include "windowtwo.h"
#include "ui_windowtwo.h"
#include <QCloseEvent>
#include <QDebug>

WindowTwo::WindowTwo(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::WindowTwo)
    , cameraThread(new CameraThread(this))
{
    ui->setupUi(this);
    setWindowTitle("窗口2");

    // 关闭按钮
    connect(ui->closeBtn, &QPushButton::clicked, this, &WindowTwo::handleClose);

    // connect(ui->setBtn, &QPushButton::clicked, this, &WindowTwo::on_setBtn_clicked);

    connect(ui->btnStart, &QPushButton::clicked, this, &WindowTwo::on_btnStart_clicked);

    connect(ui->btnStop, &QPushButton::clicked, this, &WindowTwo::on_btnStop_clicked);

    connect(cameraThread, &CameraThread::frameReady, this, &WindowTwo::updateFrame);
}


WindowTwo::~WindowTwo()
{
    // 检查相机线程是否正在运行
    if (cameraThread->isRunning()) {
        cameraThread->stop();  // 发送停止信号
        cameraThread->wait();  // 等待线程完全停止
    }
    delete ui;
}

void WindowTwo::handleClose()
{
    emit windowClosed();
    this->close();
}

void WindowTwo::on_btnStart_clicked()
{
    // 尝试打开索引为0的相机设备
    if (!cameraThread->openCamera(selectedCamera)) {
        // 如果打开失败，在相机显示标签上显示错误信息
        ui->cameraLabel->setText("❌ 无法打开摄像头");
        return;
    }
    // 打开成功，启动相机线程
    cameraThread->start();
}

void WindowTwo::on_btnStop_clicked()
{
    // 检查相机线程是否正在运行
    if (cameraThread->isRunning()) {
        cameraThread->stop();  // 发送停止信号
        cameraThread->wait();  // 等待线程完全停止
    }
    // 清除相机显示区域并显示状态信息
    ui->cameraLabel->clear();
    ui->cameraLabel->setText("摄像头已关闭");
}

void WindowTwo::on_setBtn_clicked()
{
    qDebug() << ">>> on_setBtn_clicked() 被调用";
    SettingDialog dlg(this);
    if (dlg.exec() == QDialog::Accepted) {
        selectedCamera = dlg.selectedCameraIndex();
        ui->labelCameraInfo->setText(QString("当前选择：摄像头 %1").arg(selectedCamera));
    }
}

void WindowTwo::updateFrame(const QImage &image)
{
    // 将QImage转换为QPixmap，并缩放到labelCamera的大小
    // Qt::KeepAspectRatio: 保持图像宽高比
    // Qt::SmoothTransformation: 使用平滑的缩放算法，提高图像质量
    ui->cameraLabel->setPixmap(
        QPixmap::fromImage(image).scaled(
            ui->cameraLabel->size(),
            Qt::KeepAspectRatio,
            Qt::SmoothTransformation)
        );
}

void WindowTwo::closeEvent(QCloseEvent *event)
{
    emit windowClosed();
    QDialog::closeEvent(event);
}
