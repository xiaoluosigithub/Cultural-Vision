#include "inference.h"
#include <regex>

// 定义通用最小值宏
#define min(a,b) (((a) < (b)) ? (a) : (b))

YOLO_V8::YOLO_V8() {
    // 空实现，可在此添加默认成员初始化
    cudaEnable = false;
}

YOLO_V8::~YOLO_V8() {
    delete session; // 手动释放 ONNX Runtime Session 对象
}

// -------------------- 图像转Tensor（模板函数） --------------------
template<typename T>
char* BlobFromImage(cv::Mat& iImg, T& iBlob) {
    int channels = iImg.channels();  // 图像通道数（一般为3）
    int imgHeight = iImg.rows;       // 图像高度
    int imgWidth = iImg.cols;        // 图像宽度

    // 遍历通道（B, G, R）
    for (int c = 0; c < channels; c++)
    {
        for (int h = 0; h < imgHeight; h++)
        {
            for (int w = 0; w < imgWidth; w++)
            {
                // 将像素值归一化到 [0,1] 并存入blob中
                // 存储顺序为通道优先：c * H * W + h * W + w
                iBlob[c * imgWidth * imgHeight + h * imgWidth + w] =
                    typename std::remove_pointer<T>::type((iImg.at<cv::Vec3b>(h, w)[c]) / 255.0f);
                    // 确保正确类型（float 或 half）
            }
        }
    }
    return RET_OK; // 返回成功状态（nullptr）
}

char* YOLO_V8::PreProcess(cv::Mat& iImg, std::vector<int> iImgSize, cv::Mat& oImg)
{
    if (iImg.channels() == 3)
    {
        oImg = iImg.clone(); // 如果是彩色图（3通道）
        cv::cvtColor(oImg, oImg, cv::COLOR_BGR2RGB); // OpenCV默认是BGR，这里转为RGB
    }
    else
    {
        // 如果是灰度图（1通道），扩展为3通道RGB
        cv::cvtColor(iImg, oImg, cv::COLOR_GRAY2RGB);
    }
    switch (modelType)
    {
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

const char* YOLO_V8::CreateSession(DL_INIT_PARAM& iParams) {
    const char* Ret = RET_OK;  // 默认返回成功（nullptr）
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
        rectConfidenceThreshold = iParams.rectConfidenceThreshold;
        iouThreshold = iParams.iouThreshold;
        imgSize = iParams.imgSize;
        modelType = iParams.modelType;

        env = Ort::Env(ORT_LOGGING_LEVEL_WARNING, "Yolo");

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

#endif // _WIN32

        // 实际加载ONNX模型，若路径或依赖错误将抛出异常
        session = new Ort::Session(env, modelPath, sessionOption);

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

        options = Ort::RunOptions{ nullptr };

        // 用于初始化显存、kernel、权重，减少第一次推理的延迟
        WarmUpSession();

        return RET_OK; // 成功
    }
    catch (const std::exception& e)
    {
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
    char* Ret = RET_OK;  // 定义返回值，默认返回 nullptr（表示成功）

    cv::Mat processedImg; // 定义图像处理后的容器
    // 对输入图像进行预处理（调整大小、颜色格式、填充等）
    PreProcess(iImg, imgSize, processedImg);

    // 判断模型类型（根据是否是 FLOAT32 / FLOAT16）
    if (modelType < 4)
    {
        // 分配 float 数组，大小 = 图像总像素 × 3 通道
        float* blob = new float[processedImg.total() * 3];

        // 将 OpenCV 图像转换为神经网络输入格式（归一化到 [0,1]）
        BlobFromImage(processedImg, blob);

        // 设置 ONNX 模型输入张量维度：NCHW = [1, 3, height, width]
        std::vector<int64_t> inputNodeDims = { 1, 3, imgSize.at(0), imgSize.at(1) };

        // 调用模板函数 TensorProcess 进行模型推理和结果解析
        TensorProcess(iImg, blob, inputNodeDims, oResult);
    }

    // 返回结果指针（RET_OK = nullptr 表示成功）
    return Ret;
}


template<typename N>
char* YOLO_V8::TensorProcess(cv::Mat& iImg, N& blob,
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

    // === 2 模型推理（执行前向计算） ===
    auto outputTensor = session->Run(
        options,                   // 运行选项
        inputNodeNames.data(),     // 输入节点名称数组
        &inputTensor,              // 输入张量
        1,                         // 输入数量
        outputNodeNames.data(),    // 输出节点名称数组
        outputNodeNames.size());   // 输出数量

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
        break;
    }
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
            std::cout << cudaEnable << std::endl;
            std::cout << "[YOLO_V8(CUDA)]: " << "Cuda warm-up cost "
                      << post_process_time << " ms. " << std::endl;
        }
    }
    // 返回成功标志
    return RET_OK;
}
