// window_two.cpp
#include "windowtwo.h"
#include "ui_windowtwo.h"
#include <QCloseEvent>

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

    // ---- 初始化状态 ----
    ui->btnStop->setEnabled(false);   // “停止检测”开始时不可用
    ui->statusLabel->setText("待机");
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
    // 尝试打开摄像头
    if (!cameraThread->openCamera(selectedCamera)) {
        // 如果打开失败，在相机显示标签上显示错误信息
        ui->cameraLabel->setText("❌ 无法打开摄像头");
        return;
    }
    // 打开成功，启动相机线程
    cameraThread->start();


    // ---- 启动识别线程 ----
    QString tempImagePath = "temp_frame.jpg";
    cv::Mat frame;
    // 从摄像头线程获取最近一帧图像数据
    if (cameraThread->getLastFrame(frame)) {
        cv::imwrite(tempImagePath.toStdString(), frame);
    }

    recognizeThread = new RecognizeImgThread(tempImagePath, labelPath, modelPath, this);
    connect(recognizeThread, &RecognizeImgThread::SigRecognizeFinish, this, &WindowTwo::onRecognizeSuccess);
    connect(recognizeThread, &RecognizeImgThread::SigRecognizeFail, this, &WindowTwo::onRecognizeFail);
    recognizeThread->start();


    // ---- 新增：开始检测后禁用其他按钮 ----
    ui->btnStart->setEnabled(false);
    ui->setBtn->setEnabled(false);
    ui->closeBtn->setEnabled(false);
    ui->btnStop->setEnabled(true);

    ui->statusLabel->setText("📷 正在检测中...");
}

void WindowTwo::on_btnStop_clicked()
{
    // 检查相机线程是否正在运行
    if (cameraThread->isRunning()) {
        cameraThread->stop();  // 发送停止信号
        cameraThread->wait();  // 等待线程完全停止
    }

    if (recognizeThread && recognizeThread->isRunning()) {
        recognizeThread->quit();
        recognizeThread->wait();
        recognizeThread = nullptr;
    }

    // 清除相机显示区域并显示状态信息
    ui->cameraLabel->clear();
    ui->cameraLabel->setText("摄像头已关闭");

    ui->resultLabel->setText("识别结果：--");

    // ---- 新增：恢复所有按钮可用状态 ----
    ui->btnStart->setEnabled(true);
    ui->setBtn->setEnabled(true);
    ui->closeBtn->setEnabled(true);
    ui->btnStop->setEnabled(false);

    ui->statusLabel->setText("✅ 检测已停止");
}

void WindowTwo::on_setBtn_clicked()
{
    // qDebug() << ">>> on_setBtn_clicked() 被调用";
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

    // 每隔一定帧保存一张图像并识别（避免过于频繁）
    static int frameCount = 0;
    frameCount++;
    if (frameCount % 30 == 0) { // 每30帧识别一次（约1秒）
        QString tempPath = "temp_frame.jpg";
        image.save(tempPath);
        if (recognizeThread == nullptr || !recognizeThread->isRunning()) {
            recognizeThread = new RecognizeImgThread(tempPath, labelPath, modelPath, this);
            connect(recognizeThread, &RecognizeImgThread::SigRecognizeFinish, this, &WindowTwo::onRecognizeSuccess);
            connect(recognizeThread, &RecognizeImgThread::SigRecognizeFail, this, &WindowTwo::onRecognizeFail);
            recognizeThread->start();
        }
    }
}

void WindowTwo::onRecognizeSuccess(QString className, float confidence)
{
    // ui->resultLabel->setText(QString("识别结果：%1 (置信度：%2%)")
    //                              .arg(className)
    //                              .arg(confidence * 100, 0, 'f', 2)); // 保留两位小数

    ui->resultLabel->setText(QString("识别结果：%1 ").arg(className));
    ui->conLabel->setText(QString("置信度 %1%").arg(confidence, 0, 'f', 2));
}

void WindowTwo::onRecognizeFail(QString errorMsg)
{
    ui->resultLabel->setText(QString("识别失败：%1").arg(errorMsg));
}

void WindowTwo::closeEvent(QCloseEvent *event)
{
    emit windowClosed();
    QDialog::closeEvent(event);
}
