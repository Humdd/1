#ifndef ONNXRUNTIME_CPU_H
#define ONNXRUNTIME_CPU_H

#include <QObject>

//onnxruntiem库
#include <OnnxRuntime_CPU/onnxruntime-win-x64-1.15.1/include/onnxruntime_cxx_api.h>
//opencv库
#include "opencv/include/opencv2/opencv.hpp"
//日志库
#include "log4cplus/qlog4cplus.h"

namespace bw::ai
{
    class OnnxRuntime_Cpu : public QObject
    {
        Q_OBJECT
    public:
        struct inferenceResulte
        {
            cv::Rect box;
            int classId;
            float confidence;
        };
    public:
        //根据传入的onnx模型的路径进行模型的加载
        explicit OnnxRuntime_Cpu(QString model_file);
        //析构函数
        ~OnnxRuntime_Cpu();
        //模型推理 reslut:接受结果 confidenceThreshold:置信度阈值 srcImg:源图
        bool inference(std::vector<OnnxRuntime_Cpu::inferenceResulte>& reslut, float confidenceThreshold,cv::Mat& srcImg);
        //获取模型路径
        QString getModelPath();
    private:
        Ort::Env* m_env = nullptr;
        Ort::Session* m_session_ = nullptr;
        std::vector<int64_t> m_output_dims;
        std::vector<std::string> m_input_node_names;
        std::vector<std::string> m_output_node_names;
        //模型输入信息
        int m_input_w;
        int m_input_h;
        //模型输出信息
        int m_output_w;
        int m_output_h;
        //模型路径
        QString m_modelPath;
    };

}


#endif // ONNXRUNTIME_CPU_H
