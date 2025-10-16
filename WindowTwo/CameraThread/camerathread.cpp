#include "CameraThread.h"

/**
 * @brief CameraThread类的构造函数
 * @param parent 父对象指针，通常为nullptr
 *
 * 初始化CameraThread对象，调用基类QThread的构造函数，
 * 并将running标志初始化为false，表示线程初始状态为停止。
 */
CameraThread::CameraThread(QObject *parent)
    : QThread(parent), running(false)  // 调用基类构造函数，初始化running为false
{
}

/**
 * @brief 打开指定索引的相机设备
 * @param index 相机设备索引，默认为0（通常是第一个摄像头）
 * @return bool 打开是否成功
 *
 * 使用OpenCV的VideoCapture对象尝试打开指定索引的相机设备。
 * 如果成功打开则返回true，否则返回false。
 */
bool CameraThread::openCamera(int index)
{
    // 尝试使用 DirectShow 后端
    cap.open(index, cv::CAP_DSHOW);
    if (!cap.isOpened()) {
        // 再尝试默认方式
        cap.open(index);
    }
    return cap.isOpened();
}


bool CameraThread::getLastFrame(cv::Mat &outFrame)
{
    QMutexLocker locker(&frameMutex);
    if (lastFrame.empty()) return false;
    lastFrame.copyTo(outFrame);
    return true;
}

/**
 * @brief 线程主函数，当调用start()时执行
 *
 * 这是线程的执行体，包含主要的相机捕获循环。
 * 持续从相机捕获帧，转换为QImage格式，并通过信号发送，
 * 直到running标志被设置为false或相机关闭。
 */
void CameraThread::run()
{
    running = true;  // 设置运行标志为true，开始捕获循环
    cv::Mat frame;   // OpenCV的Mat对象，用于存储从相机捕获的帧

    // 主循环：当running为true且相机处于打开状态时持续执行
    while (running && cap.isOpened()) {
        cap >> frame;  // 从相机捕获一帧到frame中（操作符重载，简化的grab和retrieve）

        // 如果捕获到的帧为空（可能由于读取错误或相机断开），跳过此次循环
        if (frame.empty()) continue;

        {
            // ✅ 保存最新帧（线程安全）
            QMutexLocker locker(&frameMutex);
            frame.copyTo(lastFrame);
        }

        // 将OpenCV的Mat格式帧转换为Qt的QImage格式
        // frame.data: 图像数据指针
        // frame.cols: 图像宽度（列数）
        // frame.rows: 图像高度（行数）
        // frame.step: 图像每行的字节数（步长）
        // QImage::Format_BGR888: 图像格式，对应OpenCV默认的BGR三通道8位格式
        QImage image(frame.data, frame.cols, frame.rows, frame.step, QImage::Format_BGR888);

        // 发送帧准备好信号，传递图像的副本（确保图像数据在线程间传递时的安全性）
        // 使用copy()创建图像的深拷贝，防止原始数据被释放后出现问题
        emit frameReady(image.copy());

        msleep(30);  // 暂停30毫秒，约等于33FPS的帧率控制
    }

    // 循环结束后，释放相机资源
    cap.release();
}

// 停止相机捕获线程
void CameraThread::stop()
{
    running = false;  // 设置运行标志为false，通知线程停止
}
