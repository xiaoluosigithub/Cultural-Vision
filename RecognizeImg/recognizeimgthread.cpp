#include "recognizeimgthread.h"

RecognizeImgThread::RecognizeImgThread(QString _img_path, QString _label_path, QString _model_path, QObject *parent) :
    _img_path(_img_path), _label_path(_label_path), _model_path(_model_path), QThread(parent)
{

}

void RecognizeImgThread::run()
{
    std::vector<std::string> classNames = readLabels(_label_path.toStdString()); // 加载类别标签
    cv::Mat image = cv::imread(_img_path.toStdString()); // 加载待识别图片
    RecognizeImg(classNames, image, _model_path); // 识别
}

std::vector<std::string> RecognizeImgThread::readLabels(const std::string &labelFile)
{
    std::vector<std::string> labels;
    std::ifstream file(labelFile);
    std::string line;

    if (!file.is_open()) {
        std::cerr << "Error: Cannot open label file: " << labelFile << std::endl;
        return labels;
    }

    while (std::getline(file, line)) {
        if (!line.empty()) {
            labels.push_back(line);
        }
    }

    std::cout << "Loaded " << labels.size() << " classes from " << labelFile << std::endl;
    return labels;
}

void RecognizeImgThread::RecognizeImg(std::vector<std::string> classNames, cv::Mat image, QString modelPath)
{
    try{
        YOLO_V8 yolo;
        DL_INIT_PARAM params;                      // 初始化参数结构体
        params.modelPath = _model_path.toStdString();  // 模型文件路径
        params.imgSize = {640, 640};                   // 模型输入图像尺寸（需与训练一致）
        params.modelType = YOLO_CLS;                   // 模型类型：YOLO 分类模型
        params.rectConfidenceThreshold = 0.01f;        // 置信度阈值（一般对分类影响不大）
        params.iouThreshold = 0.5f;                    // IoU 阈值（主要用于检测任务，这里保留默认值）
        params.cudaEnable = false;                     // 是否启用 GPU 加速（false 表示仅使用 CPU）
        params.intraOpNumThreads = 4;                  // 推理使用的线程数
        params.logSeverityLevel = 3;                   // 日志等级（3 表示仅输出错误和警告）

        // 创建模型推理会话
        const char* ret = yolo.CreateSession(params);
        if (ret != RET_OK) {
            // 如果模型创建失败，发送信号并退出线程
            emit SigRecognizeFail(QString("CreateSession failed: %1").arg(ret));
            return;
        }

        std::vector<DL_RESULT> results;
        ret = yolo.RunSession(image, results);
        if (ret != RET_OK) {
            // 如果推理失败，发送失败信号
            emit SigRecognizeFail(QString("RunSession failed: %1").arg(ret));
            return;
        }

        if (results.empty()) {
            emit SigRecognizeFail("No classification results.");
            return;
        }

        // 按置信度从高到低排序
        std::sort(results.begin(), results.end(),
                  [](const DL_RESULT &a, const DL_RESULT &b) {
                      return a.confidence > b.confidence;
                  });

        // 取排序后第一项（Top-1）
        int topId = results[0].classId;        // 类别 ID
        float topConf = results[0].confidence; // 置信度

        // 根据 ID 获取标签名，如果越界则提示无效
        QString className = (topId >= 0 && topId < (int)classNames.size())
                                ? QString::fromStdString(classNames[topId])
                                : QString("Invalid ID %1").arg(topId);
        // 发出结果信号
        emit SigRecognizeFinish(className, topConf);
    }
    catch (const std::exception &e) {
        emit SigRecognizeFail(QString("Exception: %1").arg(e.what()));
    }
}
