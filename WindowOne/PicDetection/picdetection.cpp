#include "picdetection.h"
#include "ui_picdetection.h"
#include <QDebug>

PicDetection::PicDetection(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::PicDetection)
{
    ui->setupUi(this);

    // 点击按钮进行识别
    connect(ui->pushButton, &QPushButton::clicked, this, &PicDetection::SlotRecognizeImg);

}

PicDetection::~PicDetection()
{
    delete ui;
}

void PicDetection::SlotUpdatePicPath(const QString &_path)
{
    _pic_path = _path;
}

void PicDetection::SlotDeletePath()
{
    _pic_path = "";
}

void PicDetection::SlotRecognizeImg()
{
    QString labelPath = "E:/Learn_Qt/project2/label/class_names.txt";
    QString modelPath = "E:/Learn_Qt/project2/model/best.onnx";

    // 若已有线程对象，先安全删除旧的
    if (_recognize_img_thread) {
        _recognize_img_thread->quit();
        _recognize_img_thread->wait();
        _recognize_img_thread->deleteLater();
        _recognize_img_thread = nullptr;
    }

    // 创建识别线程，父对象为当前窗口（Qt 自动管理生命周期）
    _recognize_img_thread = new RecognizeImgThread(_pic_path, labelPath, modelPath, this);

    // ===== 成功信号连接 =====
    connect(_recognize_img_thread, &RecognizeImgThread::SigRecognizeFinish,
            this, [this](const QString &className, float confidence) {
                qDebug()<< "class name = " << className << ", confidence = " << confidence << Qt::endl;
            });

    // ===== 失败信号连接 =====
    connect(_recognize_img_thread, &RecognizeImgThread::SigRecognizeFail,
            this, [this](const QString &msg) {
                qDebug()<< "faild, : " << msg << Qt::endl;
            });

    // 启动线程
    _recognize_img_thread->start();
}


