#ifndef CAMERATHREAD_H
#define CAMERATHREAD_H

// 包含必要的头文件
#include <QThread>        // Qt线程基类
#include <QImage>         // Qt图像类，用于在UI线程中显示图像
#include <opencv2/opencv.hpp>  // OpenCV库，用于相机捕获和图像处理

/**
 * @class CameraThread
 * @brief 相机捕获线程类，继承自QThread
 *
 * 该类在一个独立的线程中运行，负责从相机设备捕获视频帧，
 * 并将捕获到的OpenCV Mat格式的帧转换为Qt QImage格式，
 * 然后通过信号发送给主线程进行显示。
 */
class CameraThread : public QThread
{
    Q_OBJECT  // 必须包含此宏以使用Qt的信号和槽机制

public:
    /**
     * @brief 构造函数
     * @param parent 父对象指针，通常为nullptr
     */
    explicit CameraThread(QObject *parent = nullptr);

    /**
     * @brief 重写QThread的run方法，线程启动时执行的函数
     *
     * 该方法包含主要的循环逻辑，持续从相机捕获帧并发送信号，
     * 直到running标志被设置为false。
     */
    void run() override;

    /**
     * @brief 停止相机捕获线程
     *
     * 设置running标志为false，通知线程停止运行。
     * 注意：实际停止可能需要等待run()方法中的循环结束。
     */
    void stop();

    /**
     * @brief 打开指定索引的相机设备
     * @param index 相机设备索引，默认为0（通常是第一个摄像头）
     * @return bool 打开是否成功
     */
    bool openCamera(int index = 0);

signals:
    /**
     * @brief 帧准备好的信号
     * @param image 捕获到的图像，已转换为QImage格式
     *
     * 当成功从相机捕获一帧并转换为QImage后，通过此信号
     * 将图像发送给连接到此信号的槽函数（通常在主线程中更新UI）。
     */
    void frameReady(const QImage &image);

private:
    cv::VideoCapture cap;  // OpenCV的视频捕获对象，用于从相机获取帧
    bool running;          // 控制线程运行的标志变量
};

#endif // CAMERATHREAD_H
