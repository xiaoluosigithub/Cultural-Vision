#ifndef RECOGNIZEIMGTHREAD_H
#define RECOGNIZEIMGTHREAD_H

#include <QThread>
#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <algorithm>  // 添加algorithm头文件
#include <opencv2/opencv.hpp>
#include "inference.h"

class RecognizeImgThread : public QThread
{
    Q_OBJECT
public:
    explicit RecognizeImgThread(QString _img_path, QString _label_path, QString _model_path, QObject *parent = nullptr);
protected:
    // 线程执行函数
    virtual void run();
private:
    QString _img_path; // 待识别图片路径
    QString _label_path; // 标签文件路径
    QString _model_path; // 模型文件路径
    std::vector<std::string> readLabels(const std::string& labelFile); // 读取标签文件
    void RecognizeImg(std::vector<std::string> classNames, cv::Mat image, QString modelPath);
signals:
    void SigRecognizeFinish(QString className, float confidence); // 推理完成信号
    void SigRecognizeFail(QString errorMsg);                      // 出错信号
};

#endif // RECOGNIZEIMGTHREAD_H


