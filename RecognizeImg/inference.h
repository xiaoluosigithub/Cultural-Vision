#pragma once

// 定义返回成功的状态（此处用nullptr表示没有错误信息）
#define    RET_OK nullptr

#ifdef _WIN32
#include <Windows.h>
#include <direct.h>
#include <io.h>
#endif

#include <QtGlobal>
#include <string>
#include <vector>
#include <cstdio>
#include <opencv2/opencv.hpp>
#include "onnxruntime_cxx_api.h"

#ifdef USE_CUDA
#include <cuda_fp16.h>
#endif


// 模型类型枚举
// 根据YOLOv8模型的不同任务类型和精度(FP32 / FP16)进行区分
enum MODEL_TYPE
{
    // 浮点32位模型
    YOLO_DETECT_V8 = 1,     // YOLOv8 目标检测模型（FP32）
    YOLO_POSE = 2,          // YOLOv8 姿态估计模型（FP32）
    YOLO_CLS = 3,           // YOLOv8 分类模型（FP32）

    // 半精度浮点模型（FP16）
    YOLO_DETECT_V8_HALF = 4, // YOLOv8 目标检测模型（FP16）
    YOLO_POSE_V8_HALF = 5,   // YOLOv8 姿态估计模型（FP16）
    YOLO_CLS_HALF = 6        // YOLOv8 分类模型（FP16）
};



// 模型初始化参数结构体
// 用于传入模型创建Session时的各种配置参数
typedef struct _DL_INIT_PARAM
{
    std::string modelPath;             // 模型文件路径（.onnx文件）
    MODEL_TYPE modelType = YOLO_DETECT_V8;  // 模型类型，默认是检测模型
    std::vector<int> imgSize = {640, 640};  // 模型输入尺寸 (宽, 高)
    float rectConfidenceThreshold = 0.6f;    // 检测框置信度阈值
    float iouThreshold = 0.5f;               // NMS的IoU阈值
    int keyPointsNum = 2;                   // 姿态估计时的关键点数量
    bool cudaEnable = false;                // 是否启用GPU(CUDA)
    int logSeverityLevel = 3;               // ONNX Runtime日志级别
    int intraOpNumThreads = 1;              // CPU线程数
} DL_INIT_PARAM;


// 模型输出结果结构体
// 用于保存每个检测目标的结果信息
typedef struct _DL_RESULT
{
    int classId;                      // 预测类别ID
    float confidence;                 // 置信度
    cv::Rect box;                     // 检测框坐标 (x, y, w, h)
    std::vector<cv::Point2f> keyPoints; // 姿态关键点坐标（仅在pose模型中使用）
} DL_RESULT;


// YOLOv8 模型类封装
// 封装ONNX Runtime推理接口、预处理、后处理、CUDA初始化等
class YOLO_V8
{
public:
    YOLO_V8();    // 构造函数：初始化环境与变量
    ~YOLO_V8();   // 析构函数：释放资源

public:
    /**
     * @brief 创建ONNX推理Session
     * @param iParams 模型初始化参数（包括路径、阈值、是否启用CUDA等）
     * @return 返回nullptr表示成功，否则返回错误信息字符串
     */
    const char* CreateSession(DL_INIT_PARAM& iParams);

    /**
     * @brief 执行一次推理
     * @param iImg 输入图像（OpenCV的Mat）
     * @param oResult 输出的推理结果（检测框、类别、关键点等）
     * @return 返回nullptr表示成功，否则返回错误信息字符串
     */
    char* RunSession(cv::Mat& iImg, std::vector<DL_RESULT>& oResult);

    /**
     * @brief 模型预热
     *        一般在启动时执行一次，用于初始化显存与计算图，减少首次推理延迟
     * @return 返回nullptr表示成功
     */
    char* WarmUpSession();

    /**
     * @brief 模板函数，用于处理输入张量（CPU或GPU）
     * @tparam N 模板类型（可以是float或半精度half）
     * @param starttime_1 计时起点（可用于统计推理耗时）
     * @param iImg 输入图像
     * @param blob 模型输入张量
     * @param inputNodeDims 模型输入维度信息
     * @param oResult 输出结果
     * @return 返回nullptr表示成功
     */
    template<typename N>
    char* TensorProcess(clock_t& starttime_1, cv::Mat& iImg, N& blob,
                        std::vector<int64_t>& inputNodeDims,
                        std::vector<DL_RESULT>& oResult);

    /**
     * @brief 图像预处理函数
     *        对输入图像进行resize、归一化、通道排列转换等操作
     * @param iImg 原始输入图像
     * @param iImgSize 模型输入尺寸
     * @param oImg 输出的预处理图像
     * @return 返回nullptr表示成功
     */
    char* PreProcess(cv::Mat& iImg, std::vector<int> iImgSize, cv::Mat& oImg);

public:
    // 分类任务中保存类别名（从class_names.txt中读取）
    std::vector<std::string> classes{};

private:
    Ort::Env env;                     // ONNX Runtime环境对象
    Ort::Session* session;            // 推理Session对象
    bool cudaEnable;                  // 是否启用CUDA
    Ort::RunOptions options;          // 运行选项
    std::vector<const char*> inputNodeNames;  // 输入节点名称
    std::vector<const char*> outputNodeNames; // 输出节点名称

    MODEL_TYPE modelType;             // 当前模型类型
    std::vector<int> imgSize;         // 模型输入尺寸
    float rectConfidenceThreshold;    // 置信度阈值
    float iouThreshold;               // IoU阈值
    float resizeScales;               // 图像缩放比例（用于恢复原图检测框）
};
