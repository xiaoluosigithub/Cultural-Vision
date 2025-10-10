#include "picdetection.h"
#include "ui_picdetection.h"

PicDetection::PicDetection(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::PicDetection)
{
    ui->setupUi(this);
    connect(ui->pushButton, &QPushButton::clicked, this, &PicDetection::SlotRecognizeImg); // 点击按钮进行识别

    // 打开样式文件（这里假设 qss 文件放在资源文件中）
    QFile qssFile(":/style/PicDetection.qss");

    // 检查文件是否成功打开（只读方式）
    if (qssFile.open(QFile::ReadOnly)) {

        // 读取整个 QSS 文件内容，并将其转换为 QString
        QString style = QLatin1String(qssFile.readAll());

        // 将样式表应用到当前对话框（this 指向 RemoveProDialog）
        this->setStyleSheet(style);

        // 关闭文件
        qssFile.close();
    }

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
    QString labelPath = LABEL_PATH;
    QString modelPath = MODEL_PATH;

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
                float displayConfidence = (confidence * 100.0f > 99.99f) ? 99.99f : confidence * 100.0f ;
                ui->label_1->setText(QString("识别结果：%1 ").arg(className));
                ui->label_2->setText(QString("置信度 %1%").arg(displayConfidence, 0, 'f', 2));
            });

    // ===== 失败信号连接 =====
    connect(_recognize_img_thread, &RecognizeImgThread::SigRecognizeFail,
            this, [this](const QString &msg) {
                qDebug()<< "faild, : " << msg << Qt::endl;
            });

    // 启动线程
    _recognize_img_thread->start();
}


