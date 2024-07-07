#include "onnxruntime_cpu.h"
#include <QDebug>

namespace bw::ai
{

OnnxRuntime_Cpu::OnnxRuntime_Cpu(QString model_file)
{
    try
    {
        m_modelPath = model_file;
        // std::string str_path = model_file.toLocal8Bit().toStdString();
        // std::wstring model_path = std::wstring(str_path.begin(),str_path.end());
        std::wstring model_path = model_file.toStdWString();
        Ort::SessionOptions session_options;
        m_env = new Ort::Env(Ort::Env(ORT_LOGGING_LEVEL_ERROR, "yolo-onnx"));
        session_options.SetGraphOptimizationLevel(ORT_ENABLE_BASIC);
        // Ort::Session session_(env, model_path.c_str(), session_options);
        //设置内部操作的线程数
        // session_options.SetIntraOpNumThreads(4);

        m_session_ = new Ort::Session(*m_env, model_path.c_str(), session_options);
        size_t numInputNodes = m_session_->GetInputCount();
        size_t numOutputNodes = m_session_->GetOutputCount();
        Ort::AllocatorWithDefaultOptions allocator;
        m_input_node_names.reserve(numInputNodes);
        // 获取输入信息
        // int input_w = 0;
        // int input_h = 0;
        for (int i = 0; i < numInputNodes; i++)
        {
            auto input_name = m_session_->GetInputNameAllocated(i, allocator);
            m_input_node_names.push_back(input_name.get());
            Ort::TypeInfo input_type_info = m_session_->GetInputTypeInfo(i);
            auto input_tensor_info = input_type_info.GetTensorTypeAndShapeInfo();
            auto input_dims = input_tensor_info.GetShape();
            m_input_w = input_dims[3];
            m_input_h = input_dims[2];
            std::string input_str = "模型输入信息:"+std::to_string(input_dims[0])+" "+std::to_string(input_dims[1])+" "+std::to_string(input_dims[2])+" "+std::to_string(input_dims[3]);
            // qDebug()<< "input_dims[0] = " << input_dims[0] << "[1]:" << input_dims[1] << "[2]:" << input_dims[2] << "[3]:" << input_dims[3];
            My_LOG(QLog4cplus::Level::l_DEBUG,input_str.c_str());
        }
        // 获取输出信息
        // int output_h = 0;
        // int output_w = 0;
        Ort::TypeInfo output_type_info = m_session_->GetOutputTypeInfo(0);
        auto output_tensor_info = output_type_info.GetTensorTypeAndShapeInfo();
        m_output_dims = output_tensor_info.GetShape();
        m_output_h = m_output_dims[1];
        m_output_w = m_output_dims[2];
        std::string output_str = "模型输出信息:"+std::to_string(m_output_dims[1])+" "+std::to_string(m_output_dims[2]);
        My_LOG(QLog4cplus::Level::l_DEBUG,output_str.c_str());
        // qDebug() << "m_output_dims[1] :  = " << m_output_dims[1] << "m_output_dims[2]" << m_output_dims[2] ;
        for (int i = 0; i < numOutputNodes; i++)
        {
            auto out_name = m_session_->GetOutputNameAllocated(i, allocator);
            m_output_node_names.push_back(out_name.get());
        }
        // qDebug() << "input: "<< m_input_node_names[0].c_str() << " output: " << m_output_node_names[0].c_str();
        session_options.release();
    } catch (...)
    {
        //构造异常
        qDebug()<<"构造异常";
        return;
    }

}

OnnxRuntime_Cpu::~OnnxRuntime_Cpu()
{
    if(nullptr!=this->m_env)
    {
        delete m_env;
        this->m_env = nullptr;
    }
    if(this->m_session_!=nullptr)
    {
        this->m_session_->release();
        delete this->m_session_;
        this->m_session_=nullptr;
    }
}

bool OnnxRuntime_Cpu::inference(std::vector<inferenceResulte> &result, float confidenceThreshold, cv::Mat &srcImg)
{
    try
    {
        std::chrono::high_resolution_clock::time_point time_start1 = std::chrono::high_resolution_clock::now();
        cv::Mat colorImg;
        if(1 == srcImg.channels())
        {
            //灰度图
            cv::cvtColor(srcImg,colorImg,cv::COLOR_GRAY2BGR);
        }
        else
        {
            colorImg = srcImg;
        }
        int w = colorImg.cols;
        int h = colorImg.rows;
        int _max = std::max(h,w);
        cv::Mat image = cv::Mat::zeros(cv::Size(_max,_max),CV_8UC3);
        cv::Rect roi(0,0,w,h);
        colorImg.copyTo(image(roi));

        float x_factor = image.cols / static_cast<float>(m_input_w);
        float y_factor = image.rows / static_cast<float>(m_input_h);
        std::chrono::high_resolution_clock::time_point end1 = std::chrono::high_resolution_clock::now();
        std::chrono::milliseconds time1 = std::chrono::duration_cast<std::chrono::milliseconds>(end1 - time_start1);
        // My_LOG(QLog4cplus::Level::l_DEBUG,std::string("内部推理时间1："+std::to_string(time1.count())+"ms").c_str());

        std::chrono::high_resolution_clock::time_point time_start2 = std::chrono::high_resolution_clock::now();
        cv::Mat blob = cv::dnn::blobFromImage(image,1/255.0,cv::Size(m_input_w,m_input_h),cv::Scalar(0,0,0),true,false);
        std::chrono::high_resolution_clock::time_point end2 = std::chrono::high_resolution_clock::now();
        std::chrono::milliseconds time2 = std::chrono::duration_cast<std::chrono::milliseconds>(end2 - time_start2);
        // My_LOG(QLog4cplus::Level::l_DEBUG,std::string("blob时间："+std::to_string(time2.count())+"ms").c_str());

        std::chrono::high_resolution_clock::time_point time_start3 = std::chrono::high_resolution_clock::now();
        size_t tpixels = m_input_h * m_input_w * 3;
        std::array<int64_t, 4>input_shape_info{1 ,3, m_input_h, m_input_w};
        auto allocator_info = Ort::MemoryInfo::CreateCpu(OrtDeviceAllocator, OrtMemTypeCPU);
        Ort::Value input_tensor_ = Ort::Value::CreateTensor<float>(allocator_info, blob.ptr<float>(), tpixels, input_shape_info.data(), input_shape_info.size());
        const std::array<const char*, 1> inputNames = { m_input_node_names[0].c_str() };
        const std::array<const char*, 1> outNames = { m_output_node_names[0].c_str() };
        std::vector<Ort::Value> ort_outputs;
        std::chrono::high_resolution_clock::time_point end3 = std::chrono::high_resolution_clock::now();
        std::chrono::milliseconds time3 = std::chrono::duration_cast<std::chrono::milliseconds>(end3 - time_start3);
        // My_LOG(QLog4cplus::Level::l_DEBUG,std::string("推理时间3："+std::to_string(time3.count())+"ms").c_str());

        try
        {
            std::chrono::high_resolution_clock::time_point time_start = std::chrono::high_resolution_clock::now();
            ort_outputs = m_session_->Run(Ort::RunOptions{ nullptr }, inputNames.data(), &input_tensor_, 1, outNames.data(), outNames.size());
            std::chrono::high_resolution_clock::time_point end = std::chrono::high_resolution_clock::now();
            std::chrono::milliseconds time = std::chrono::duration_cast<std::chrono::milliseconds>(end - time_start);
            // My_LOG(QLog4cplus::Level::l_DEBUG,std::string("内部推理时间："+std::to_string(time.count())+"ms").c_str());
        }
        catch (std::exception e)
        {
            qDebug() << e.what() ;
            return false;
        }
        const float* pdata = ort_outputs[0].GetTensorMutableData<float>();

        //yolov5推理结果解析
        int num_detections = m_output_dims[1];
        std::vector<cv::Rect> boxes;
        std::vector<int> classIds;
        std::vector<float> confidences;
        for(int i=0;i<num_detections;++i)
        {
            float x_center = pdata[i * m_output_dims[2] + 0];
            float y_center = pdata[i * m_output_dims[2] + 1];
            float width = pdata[i * m_output_dims[2] + 2];
            float height = pdata[i * m_output_dims[2] + 3];
            float confidence = pdata[i * m_output_dims[2] + 4];
            // int classId = static_cast<int>(pdata[i * m_output_dims[2] + 5]);
            int classId;
            float maxProb = 0;
            for(int classIndex=0;classIndex<m_output_dims[2]-5;++classIndex)
            {
                float prob = pdata[i * m_output_dims[2] + 5 + classIndex]; // 5为前5个属性之后的类别概率起始位置
                if (prob > maxProb)
                {
                    maxProb = prob;
                    classId = classIndex;
                }
            }
            if(confidence > confidenceThreshold)
            {
                int x = static_cast<int>((x_center - 0.5 * width) * x_factor);
                int y = static_cast<int>((y_center - 0.5 * height) * y_factor);
                int w = static_cast<int>(width * x_factor);
                int h = static_cast<int>(height * y_factor);
                cv::Rect box(x, y, w, h);
                boxes.push_back(box);
                classIds.push_back(classId);
                confidences.push_back(confidence);
            }
        }
        //NMS
        std::vector<int> indexes;
        cv::dnn::NMSBoxes(boxes, confidences, 0.25, 0.45, indexes);
        for(int i=0;i<indexes.size();++i)
        {
            int index = indexes[i];
            OnnxRuntime_Cpu::inferenceResulte R;
            R.box = boxes[index];
            R.classId = classIds[index];
            R.confidence = confidences[index];
            result.emplace_back(R);
        }
        //推理成功
        return true;
    } catch (...)
    {
        //推理异常
        return false;
    }

}

QString OnnxRuntime_Cpu::getModelPath()
{
    return m_modelPath;
}


}
















