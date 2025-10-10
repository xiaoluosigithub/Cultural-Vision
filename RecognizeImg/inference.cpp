#include "inference.h"
#include <regex>

// 启用benchmark模式（如果定义了，可用于后续性能测试）
#define benchmark

// 定义通用最小值宏
#define min(a,b) (((a) < (b)) ? (a) : (b))

// -------------------- 构造与析构 --------------------

/**
 * @brief YOLO_V8 类构造函数
 *
 * 用于初始化类成员变量。此处暂未在构造函数中初始化 ONNX Runtime 相关资源，
 * 因为这些资源会在 CreateSession() 中根据参数动态创建。
 */
YOLO_V8::YOLO_V8() {
    // 空实现，可在此添加默认成员初始化
}


/**
 * @brief YOLO_V8 类析构函数
 *
 * 析构时释放 Session 指针，防止内存泄漏。
 */
YOLO_V8::~YOLO_V8() {
    delete session; // 手动释放 ONNX Runtime Session 对象
}


// -------------------- 半精度类型支持 --------------------

#ifdef USE_CUDA
namespace Ort
{
/**
 * @brief 模板特化：定义 ONNX Runtime 对 half 类型的映射
 *
 * 由于 ONNX Runtime 的类型系统中默认没有 C++ 的 `half` 类型（即 float16），
 * 因此这里通过模板特化定义 `TypeToTensorType<half>` 的映射关系，
 * 使得在创建张量时可以正确识别半精度浮点数据。
 */
template<>
struct TypeToTensorType<half> {
    static constexpr ONNXTensorElementDataType type = ONNX_TENSOR_ELEMENT_DATA_TYPE_FLOAT16;
};
}
#endif


// -------------------- 图像转Tensor（模板函数） --------------------

/**
 * @brief 将输入图像转换为模型输入张量格式（NCHW）
 *
 * 这是一个模板函数，用于将 OpenCV 的 `cv::Mat` 图像数据拷贝到模型输入的内存缓冲区中。
 * 模板参数 T 代表输入数据类型（可以是 float 或 half），用于支持 FP32 与 FP16 两种模型。
 *
 * @tparam T 模型输入数据类型（float* 或 half*）
 * @param iImg 输入的 OpenCV 图像 (BGR通道)
 * @param iBlob 模型输入张量的内存指针（一般由调用者分配）
 * @return char* 返回状态码，nullptr 表示成功
 *
 * @note
 * - 输出张量格式为 NCHW（通道优先）
 * - 图像会被归一化到 [0,1]，即除以255
 * - 通道顺序为 BGR（与OpenCV一致）
 */
template<typename T>
char* BlobFromImage(cv::Mat& iImg, T& iBlob) {
    int channels = iImg.channels();  // 图像通道数（一般为3）
    int imgHeight = iImg.rows;       // 图像高度
    int imgWidth = iImg.cols;        // 图像宽度

    // 遍历通道（B, G, R）
    for (int c = 0; c < channels; c++)
    {
        // 遍历每一行
        for (int h = 0; h < imgHeight; h++)
        {
            // 遍历每一列
            for (int w = 0; w < imgWidth; w++)
            {
                // 将像素值归一化到 [0,1] 并存入blob中
                // 存储顺序为通道优先：c * H * W + h * W + w
                iBlob[c * imgWidth * imgHeight + h * imgWidth + w] =
                    typename std::remove_pointer<T>::type(   // 确保正确类型（float 或 half）
                        (iImg.at<cv::Vec3b>(h, w)[c]) / 255.0f
                        );
            }
        }
    }

    return RET_OK; // 返回成功状态（nullptr）
}


/**
 * @brief 图像预处理函数
 *
 * 该函数根据不同模型类型（检测 / 姿态 / 分类）对输入图像进行适配：
 * - 对输入图像进行颜色空间转换（BGR → RGB）
 * - 根据模型类型执行不同的预处理策略：
 *   - YOLO 检测 / 姿态模型：LetterBox 填充（保持长宽比）
 *   - YOLO 分类模型：CenterCrop 居中裁剪
 *
 * 预处理的目标是生成与模型输入尺寸一致的 `oImg`。
 *
 * @param iImg     原始输入图像（BGR 或灰度）
 * @param iImgSize 模型输入尺寸（通常为 [640, 640]）
 * @param oImg     输出的预处理后图像（RGB格式）
 * @return char*   返回 nullptr 表示成功
 */
char* YOLO_V8::PreProcess(cv::Mat& iImg, std::vector<int> iImgSize, cv::Mat& oImg)
{
    // ---------- Step 1: 颜色空间转换 ----------
    if (iImg.channels() == 3)
    {
        // 如果是彩色图（3通道）
        oImg = iImg.clone();
        cv::cvtColor(oImg, oImg, cv::COLOR_BGR2RGB); // OpenCV默认是BGR，这里转为RGB
    }
    else
    {
        // 如果是灰度图（1通道），扩展为3通道RGB
        cv::cvtColor(iImg, oImg, cv::COLOR_GRAY2RGB);
    }

    // ---------- Step 2: 不同模型类型对应不同的预处理策略 ----------
    switch (modelType)
    {
    // ------------------ 检测模型 & 姿态模型 ------------------
    case YOLO_DETECT_V8:
    case YOLO_POSE:
    case YOLO_DETECT_V8_HALF:
    case YOLO_POSE_V8_HALF:
    {
        // 使用 LetterBox 缩放策略（保持长宽比不变）
        // LetterBox：指在缩放后，用黑边填充使图像达到模型输入尺寸

        if (iImg.cols >= iImg.rows)
        {
            // 如果图像更宽（横向长）
            resizeScales = iImg.cols / (float)iImgSize.at(0); // 计算缩放比例
            // 按比例缩放图像（宽固定为模型输入宽）
            cv::resize(oImg, oImg, cv::Size(iImgSize.at(0), int(iImg.rows / resizeScales)));
        }
        else
        {
            // 如果图像更高（纵向长）
            resizeScales = iImg.rows / (float)iImgSize.at(0); // 计算缩放比例
            // 按比例缩放图像（高固定为模型输入高）
            cv::resize(oImg, oImg, cv::Size(int(iImg.cols / resizeScales), iImgSize.at(1)));
        }

        // 创建一张目标尺寸的黑色背景图（填充区域）
        cv::Mat tempImg = cv::Mat::zeros(iImgSize.at(0), iImgSize.at(1), CV_8UC3);

        // 将缩放后的图像复制到左上角位置
        oImg.copyTo(tempImg(cv::Rect(0, 0, oImg.cols, oImg.rows)));

        // 最终输出图像为完整的 LetterBox 填充结果
        oImg = tempImg;
        break;
    }

    // ------------------ 分类模型 ------------------
    case YOLO_CLS:
    {
        // 使用 CenterCrop 策略（居中裁剪）
        // 在分类模型中，输入尺寸通常固定为方形，因此直接裁剪中心区域即可

        int h = iImg.rows; // 原图高
        int w = iImg.cols; // 原图宽
        int m = min(h, w); // 取较短边作为裁剪边长

        // 计算中心裁剪区域的左上角坐标
        int top = (h - m) / 2;
        int left = (w - m) / 2;

        // 从原图中裁剪出中心方形区域，并缩放到模型输入尺寸
        cv::resize(oImg(cv::Rect(left, top, m, m)), oImg, cv::Size(iImgSize.at(0), iImgSize.at(1)));

        break;
    }
    }

    // 返回成功状态
    return RET_OK;
}



/**
 * @brief 创建 YOLOv8 推理 Session
 *
 * 该函数负责加载 ONNX 模型、配置运行参数、初始化推理环境等。
 * 根据输入参数（DL_INIT_PARAM）可选择启用 CPU 或 CUDA 模式。
 *
 * @param iParams 模型初始化参数（路径、类型、阈值、是否启用GPU等）
 * @return const char* 返回状态字符串：
 *         - nullptr 表示成功
 *         - 非 nullptr 表示错误信息
 */
const char* YOLO_V8::CreateSession(DL_INIT_PARAM& iParams) {
    const char* Ret = RET_OK;  // 默认返回成功（nullptr）

    // ---------- Step 1：检查模型路径中是否包含中文字符 ----------
    // ONNX Runtime 在 Windows 下可能无法正确解析含中文路径，故需检测
    std::regex pattern("[\u4e00-\u9fa5]");
    bool result = std::regex_search(iParams.modelPath, pattern);
    if (result)
    {
        Ret = "[YOLO_V8]:Your model path is error. Change your model path without chinese characters.";
        std::cout << Ret << std::endl;
        return Ret;  // 路径包含中文，直接返回错误提示
    }

    try
    {
        // ---------- Step 2：保存外部参数到类成员 ----------
        rectConfidenceThreshold = iParams.rectConfidenceThreshold;
        iouThreshold = iParams.iouThreshold;
        imgSize = iParams.imgSize;
        modelType = iParams.modelType;

        // ---------- Step 3：创建 ONNX Runtime 环境 ----------
        // ORT_LOGGING_LEVEL_WARNING：设置日志级别为警告（减少输出噪声）
        env = Ort::Env(ORT_LOGGING_LEVEL_WARNING, "Yolo");

        // ---------- Step 4：配置 Session 选项 ----------
        Ort::SessionOptions sessionOption;

        // 若启用 CUDA，则添加 CUDA 执行提供者
        if (iParams.cudaEnable)
        {
            cudaEnable = iParams.cudaEnable;

            OrtCUDAProviderOptions cudaOption;
            cudaOption.device_id = 0;  // 使用 GPU 0
            sessionOption.AppendExecutionProvider_CUDA(cudaOption);
        }

        // 开启所有图优化（算子融合、常量折叠等）
        sessionOption.SetGraphOptimizationLevel(GraphOptimizationLevel::ORT_ENABLE_ALL);

        // 设置并行线程数（CPU模式下生效）
        sessionOption.SetIntraOpNumThreads(iParams.intraOpNumThreads);

        // 设置日志严重级别（0=verbose, 1=info, 2=warning, 3=error, 4=fatal）
        sessionOption.SetLogSeverityLevel(iParams.logSeverityLevel);


        // ---------- Step 5：加载模型文件 ----------
#ifdef _WIN32
        // Windows下ONNX要求使用宽字符路径，因此进行UTF-8 → UTF-16转换
        int ModelPathSize = MultiByteToWideChar(
            CP_UTF8, 0,
            iParams.modelPath.c_str(),
            static_cast<int>(iParams.modelPath.length()),
            nullptr, 0
            );

        wchar_t* wide_cstr = new wchar_t[ModelPathSize + 1];
        MultiByteToWideChar(
            CP_UTF8, 0,
            iParams.modelPath.c_str(),
            static_cast<int>(iParams.modelPath.length()),
            wide_cstr, ModelPathSize
            );
        wide_cstr[ModelPathSize] = L'\0';
        const wchar_t* modelPath = wide_cstr;
#else
        // Linux下可直接使用UTF-8路径
        const char* modelPath = iParams.modelPath.c_str();
#endif // _WIN32


        // ---------- Step 6：创建 ONNX Runtime Session ----------
        // 实际加载ONNX模型，若路径或依赖错误将抛出异常
        session = new Ort::Session(env, modelPath, sessionOption);

        // ---------- Step 7：获取输入与输出节点名称 ----------

        Ort::AllocatorWithDefaultOptions allocator; // 创建分配器

        // 获取输入节点数量
        size_t inputNodesNum = session->GetInputCount();
        // 遍历输入节点
        for (size_t i = 0; i < inputNodesNum; i++)
        {
            // 获取输入节点名称
            Ort::AllocatedStringPtr input_node_name = session->GetInputNameAllocated(i, allocator);

            // 分配内存并复制名称
            char* temp_buf = new char[50];
            strcpy_s(temp_buf, sizeof(temp_buf), input_node_name.get());
            inputNodeNames.push_back(temp_buf);
        }

        // 获取输出节点数量
        size_t OutputNodesNum = session->GetOutputCount();
        for (size_t i = 0; i < OutputNodesNum; i++)
        {
            // 获取输出节点名称
            Ort::AllocatedStringPtr output_node_name = session->GetOutputNameAllocated(i, allocator);

            // 分配内存并复制名称
            char* temp_buf = new char[10];
            strcpy_s(temp_buf, sizeof(temp_buf), output_node_name.get());
            outputNodeNames.push_back(temp_buf);
        }

        // ---------- Step 8：初始化运行选项 ----------
        options = Ort::RunOptions{ nullptr };

        // ---------- Step 9：预热模型 ----------
        // 用于初始化显存、kernel、权重，减少第一次推理的延迟
        WarmUpSession();

        return RET_OK; // 成功
    }
    catch (const std::exception& e)
    {
        // ---------- Step 10：捕获异常并返回错误信息 ----------
        const char* str1 = "[YOLO_V8]:";
        const char* str2 = e.what();
        std::string result = std::string(str1) + std::string(str2);

        // 输出详细错误信息到控制台
        char* merged = new char[result.length() + 1];
        strcpy_s(merged, sizeof(merged), result.c_str());
        std::cout << merged << std::endl;
        delete[] merged;

        // 返回简短错误提示
        return "[YOLO_V8]:Create session failed.";
    }
}



char* YOLO_V8::RunSession(cv::Mat& iImg, std::vector<DL_RESULT>& oResult) {
#ifdef benchmark
    // 如果定义了 benchmark 宏，则开始计时，用于性能测试
    clock_t starttime_1 = clock();
#endif // benchmark

    char* Ret = RET_OK;  // 定义返回值，默认返回 nullptr（表示成功）

    cv::Mat processedImg; // 定义图像处理后的容器
    // 对输入图像进行预处理（调整大小、颜色格式、填充等）
    PreProcess(iImg, imgSize, processedImg);

    // 判断模型类型（根据是否是 FLOAT32 / FLOAT16）
    if (modelType < 4) // 即：YOLO_DETECT_V8、YOLO_POSE、YOLO_CLS（FP32 模型）
    {
        // 分配 float 数组，大小 = 图像总像素 × 3 通道
        float* blob = new float[processedImg.total() * 3];

        // 将 OpenCV 图像转换为神经网络输入格式（归一化到 [0,1]）
        BlobFromImage(processedImg, blob);

        // 设置 ONNX 模型输入张量维度：NCHW = [1, 3, height, width]
        std::vector<int64_t> inputNodeDims = { 1, 3, imgSize.at(0), imgSize.at(1) };

        // 调用模板函数 TensorProcess 进行模型推理和结果解析
        TensorProcess(starttime_1, iImg, blob, inputNodeDims, oResult);
    }
    else // 如果是 FLOAT16 模型（半精度），如 YOLO_DETECT_V8_HALF / YOLO_POSE_V8_HALF / YOLO_CLS_HALF
    {
#ifdef USE_CUDA
        // 分配 half 精度（float16）数组
        half* blob = new half[processedImg.total() * 3];

        // 同样将图像数据转换为模型输入格式（不过是半精度类型）
        BlobFromImage(processedImg, blob);

        // 设置输入维度（与上面相同）
        std::vector<int64_t> inputNodeDims = { 1, 3, imgSize.at(0), imgSize.at(1) };

        // 调用推理函数，进行 GPU 模型推理
        TensorProcess(starttime_1, iImg, blob, inputNodeDims, oResult);
#endif
    }

    // 返回结果指针（RET_OK = nullptr 表示成功）
    return Ret;
}


template<typename N>
char* YOLO_V8::TensorProcess(clock_t& starttime_1, cv::Mat& iImg, N& blob,
                             std::vector<int64_t>& inputNodeDims, std::vector<DL_RESULT>& oResult)
{
    Q_UNUSED(iImg);
    // === 1️ 创建 ONNX Runtime 输入张量 ===
    Ort::Value inputTensor = Ort::Value::CreateTensor<typename std::remove_pointer<N>::type>(
        Ort::MemoryInfo::CreateCpu(OrtDeviceAllocator, OrtMemTypeCPU),  // 在 CPU 上创建内存
        blob,                                                           // 输入数据指针
        3 * imgSize.at(0) * imgSize.at(1),                              // 输入张量元素数量
        inputNodeDims.data(),                                           // 输入维度数组
        inputNodeDims.size());                                          // 维度数量

#ifdef benchmark
    // 记录推理前时间点（用于计算前处理时间）
    clock_t starttime_2 = clock();
#endif // benchmark

    // === 2 模型推理（执行前向计算） ===
    auto outputTensor = session->Run(
        options,                   // 运行选项
        inputNodeNames.data(),     // 输入节点名称数组
        &inputTensor,              // 输入张量
        1,                         // 输入数量
        outputNodeNames.data(),    // 输出节点名称数组
        outputNodeNames.size());   // 输出数量

#ifdef benchmark
    // 记录推理后时间点（用于计算推理时间）
    clock_t starttime_3 = clock();
#endif // benchmark


    // === 3️ 获取输出张量信息 ===
    Ort::TypeInfo typeInfo = outputTensor.front().GetTypeInfo();           // 获取输出类型信息
    auto tensor_info = typeInfo.GetTensorTypeAndShapeInfo();               // 获取形状信息
    std::vector<int64_t> outputNodeDims = tensor_info.GetShape();          // 输出张量维度
    auto output = outputTensor.front().GetTensorMutableData<typename std::remove_pointer<N>::type>(); // 输出数据指针

    // 释放输入内存（blob 在推理后已不再使用）
    delete[] blob;


    // === 4️ 根据模型类型解析输出 ===
    switch (modelType)
    {
    // --------------------【检测模型 YOLOv8】--------------------
    case YOLO_DETECT_V8:
    case YOLO_DETECT_V8_HALF:
    {
        int strideNum = static_cast<int>(outputNodeDims[1]);      // 通常为 8400（anchor 数量）
        int signalResultNum = static_cast<int>(outputNodeDims[2]); // 每个预测结果的维度（例如 84 = 4+80）
        std::vector<int> class_ids;
        std::vector<float> confidences;
        std::vector<cv::Rect> boxes;
        cv::Mat rawData;

        // FP32 与 FP16 的处理方式不同
        if (modelType == YOLO_DETECT_V8)
        {
            // float32 模型
            rawData = cv::Mat(strideNum, signalResultNum, CV_32F, output);
        }
        else
        {
            // float16 模型：先转成 FP16 Mat，再转换成 FP32
            rawData = cv::Mat(strideNum, signalResultNum, CV_16F, output);
            rawData.convertTo(rawData, CV_32F);
        }

        // 由于 Ultralytics 的 YOLOv8 模型对输出做了转置（transpose），所以输出 shape 与 v5/v7 一致
        // 参考：https://github.com/ultralytics/assets/releases/download/v8.2.0/yolov8n.pt
        // rawData = rawData.t();

        float* data = (float*)rawData.data;

        // === 遍历每一个检测结果 ===
        for (int i = 0; i < strideNum; ++i)
        {
            float* classesScores = data + 4; // 前4个是 box，后面的为类别置信度
            cv::Mat scores(1, static_cast<int>(this->classes.size()), CV_32FC1, classesScores);

            // 获取置信度最高的类别
            cv::Point class_id;
            double maxClassScore;
            cv::minMaxLoc(scores, 0, &maxClassScore, 0, &class_id);

            // 若置信度超过阈值，保留该检测框
            if (maxClassScore > rectConfidenceThreshold)
            {
                confidences.push_back(static_cast<float>(maxClassScore));
                class_ids.push_back(class_id.x);

                // 获取预测的 (x,y,w,h)
                float x = data[0];
                float y = data[1];
                float w = data[2];
                float h = data[3];

                // 将相对坐标还原为原图坐标
                int left = int((x - 0.5 * w) * resizeScales);
                int top = int((y - 0.5 * h) * resizeScales);
                int width = int(w * resizeScales);
                int height = int(h * resizeScales);

                // 保存检测框
                boxes.push_back(cv::Rect(left, top, width, height));
            }

            // 跳到下一个预测
            data += signalResultNum;
        }

        // === 执行 NMS（非极大值抑制）去除重叠框 ===
        std::vector<int> nmsResult;
        cv::dnn::NMSBoxes(boxes, confidences, rectConfidenceThreshold, iouThreshold, nmsResult);

        // === 保存最终结果 ===
        for (int i = 0; i < nmsResult.size(); ++i)
        {
            int idx = nmsResult[i];
            DL_RESULT result;
            result.classId = class_ids[idx];
            result.confidence = confidences[idx];
            result.box = boxes[idx];
            oResult.push_back(result);
        }

#ifdef benchmark
        // === 打印性能信息 ===
        clock_t starttime_4 = clock();
        double pre_process_time = (double)(starttime_2 - starttime_1) / CLOCKS_PER_SEC * 1000;
        double process_time = (double)(starttime_3 - starttime_2) / CLOCKS_PER_SEC * 1000;
        double post_process_time = (double)(starttime_4 - starttime_3) / CLOCKS_PER_SEC * 1000;

        if (cudaEnable)
        {
            std::cout << "[YOLO_V8(CUDA)]: " << pre_process_time << "ms pre-process, "
                      << process_time << "ms inference, " << post_process_time << "ms post-process." << std::endl;
        }
        else
        {
            std::cout << "[YOLO_V8(CPU)]: " << pre_process_time << "ms pre-process, "
                      << process_time << "ms inference, " << post_process_time << "ms post-process." << std::endl;
        }
#endif // benchmark

        break;
    }

    // --------------------【分类模型 YOLOv8-CLS】--------------------
    case YOLO_CLS:
    {
        cv::Mat rawData;
        rawData = cv::Mat(1, static_cast<int>(outputNodeDims.back()), CV_32F, output);

        float* data = (float*)rawData.data;
        int num_classes = static_cast<int>(outputNodeDims.back());

        // === 找出最大置信度类别 ===
        int max_index = 0;
        float max_value = data[0];
        for (int i = 1; i < num_classes; ++i)
        {
            if (data[i] > max_value)
            {
                max_value = data[i];
                max_index = i;
            }
        }

        // 保存结果
        DL_RESULT result;
        result.classId = max_index;
        result.confidence = max_value;
        oResult.push_back(result);

        // 输出预测结果
        if (!classes.empty() && max_index < classes.size())
            std::cout << "[YOLO_V8]: Predicted class = " << classes[max_index]
                      << ", confidence = " << max_value << std::endl;
        else
            std::cout << "[YOLO_V8]: Predicted class id = " << max_index
                      << ", confidence = " << max_value << std::endl;

        break;
    }

    // --------------------【未支持模型类型】--------------------
    default:
        std::cout << "[YOLO_V8]: Not support model type." << std::endl;
    }

    // 返回执行成功标志
    return RET_OK;
}



// 模型预热函数（WarmUpSession）
// 作用：在真正推理前先运行一次模型，用于CUDA/CPU环境的初始化，避免第一次推理时延迟过高。
char* YOLO_V8::WarmUpSession() {
    // 记录起始时间，用于计算预热耗时
    clock_t starttime_1 = clock();

    // 创建一个空白图像（填充为零的彩色图像）
    // 图像大小与模型输入尺寸一致，例如640x640x3
    cv::Mat iImg = cv::Mat(cv::Size(imgSize.at(0), imgSize.at(1)), CV_8UC3);

    // 定义一个用于保存预处理结果的图像
    cv::Mat processedImg;

    // 对输入图像进行预处理（尺寸缩放、通道转换、归一化等）
    PreProcess(iImg, imgSize, processedImg);

    // 根据模型类型判断是 float 模型还是 half 精度模型
    if (modelType < 4)  // YOLOv8 float 模型
    {
        // 为输入张量分配 float 类型缓冲区（每像素3通道）
        float* blob = new float[iImg.total() * 3];

        // 将图像数据转换成模型输入张量所需的格式（例如CHW顺序、归一化等）
        BlobFromImage(processedImg, blob);

        // 定义输入张量的维度：[1, 3, H, W]
        std::vector<int64_t> YOLO_input_node_dims = { 1, 3, imgSize.at(0), imgSize.at(1) };

        // 创建 ONNX Runtime 输入张量对象
        Ort::Value input_tensor = Ort::Value::CreateTensor<float>(
            Ort::MemoryInfo::CreateCpu(OrtDeviceAllocator, OrtMemTypeCPU),
            blob,
            3 * imgSize.at(0) * imgSize.at(1),
            YOLO_input_node_dims.data(),
            YOLO_input_node_dims.size()
            );

        // 执行一次模型推理，实际不关心输出，只用于激活 CUDA/CPU 内核
        auto output_tensors = session->Run(
            options,
            inputNodeNames.data(), &input_tensor, 1,
            outputNodeNames.data(), outputNodeNames.size()
            );

        // 释放内存
        delete[] blob;

        // 计算预热总耗时（毫秒）
        clock_t starttime_4 = clock();
        double post_process_time = (double)(starttime_4 - starttime_1) / CLOCKS_PER_SEC * 1000;

        // 如果开启CUDA模式，打印耗时信息
        if (cudaEnable)
        {
            std::cout << "[YOLO_V8(CUDA)]: " << "Cuda warm-up cost "
                      << post_process_time << " ms. " << std::endl;
        }
    }
    else  // YOLOv8 半精度（FP16）模型
    {
#ifdef USE_CUDA
        // 为输入张量分配 half 精度缓冲区
        half* blob = new half[iImg.total() * 3];

        // 将图像数据转换为 half 格式输入
        BlobFromImage(processedImg, blob);

        // 定义输入张量维度：[1, 3, H, W]
        std::vector<int64_t> YOLO_input_node_dims = { 1, 3, imgSize.at(0), imgSize.at(1) };

        // 创建 half 精度输入张量
        Ort::Value input_tensor = Ort::Value::CreateTensor<half>(
            Ort::MemoryInfo::CreateCpu(OrtDeviceAllocator, OrtMemTypeCPU),
            blob,
            3 * imgSize.at(0) * imgSize.at(1),
            YOLO_input_node_dims.data(),
            YOLO_input_node_dims.size()
            );

        // 执行模型推理
        auto output_tensors = session->Run(
            options,
            inputNodeNames.data(), &input_tensor, 1,
            outputNodeNames.data(), outputNodeNames.size()
            );

        // 释放内存
        delete[] blob;

        // 计算预热耗时
        clock_t starttime_4 = clock();
        double post_process_time = (double)(starttime_4 - starttime_1) / CLOCKS_PER_SEC * 1000;

        // 打印CUDA预热耗时
        if (cudaEnable)
        {
            std::cout << "[YOLO_V8(CUDA)]: " << "Cuda warm-up cost "
                      << post_process_time << " ms. " << std::endl;
        }
#endif
    }

    // 返回成功标志
    return RET_OK;
}
