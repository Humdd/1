#include "mainwindow.h"
#include <QApplication>
#include <QFile>
#include <QJsonDocument>
#include <QJsonArray>
#include <algorithm>
#include <iostream>
#include <chrono>
#include "opencv2/opencv.hpp"
#include "utils.h"
#include <QElapsedTimer>
using namespace chrono;

// 主程序
int main(int argc, char *argv[])
{
    //quintptr是Qt中的一种类型，用于表示指针或句柄等无符号整数类型的值;这段代码使用了Qt的QThread类中的currentThreadId()函数，该函数返回当前线程的唯一标识符，类型为quintptr。然后使用QString的arg()函数将这个唯一标识符插入到字符串 "main() threadId: %1" 中，生成一个新的QString字符串。最后，使用Q_LOG4CPLUS_INFO宏输出这个字符串作为日志信息。
    Q_LOG4CPLUS_INFO(QString("main() threadId: %1").arg(quintptr(QThread::currentThreadId())));
    //创建应用程序对象
    QApplication a(argc, argv);
    MainWindow w;
    w.show();
    //应用程序循环
    return a.exec();
}

void test_logo()
{
    //读取原图
    QString img_path("./images/0008_1.bmp");
    QString fileName = QFileInfo(img_path).baseName();
    cv::Mat mat = cv::imread(img_path.toStdString(), cv::IMREAD_GRAYSCALE);
    QImage image(mat.data, mat.cols, mat.rows, mat.step, QImage::Format_Grayscale8); //image.format()底层数字是24
    if (mat.empty())
    {
        std::cerr << "mat.empty()" << std::endl;
        return;
    }
    if (image.isNull())
    {
        std::cerr << "image.isNull()" << std::endl;
        return;
    }

    //获取logo
    cv::Rect rect_logo = cv::Rect(1716, 1309, 141, 486);
    cv::Mat mat_logo = mat(rect_logo);
    cv::imshow("mat_logo", mat_logo);
//    cv::waitKey(0);

    //读取模板templ
    cv::Mat templ = cv::imread("./images/templ/0027_1.bmp", cv::IMREAD_GRAYSCALE);
    cv::imwrite("./images/templ/templ.bmp", templ);
    cv::Mat templ_bin;
    cv::threshold(templ, templ_bin, 0, 255, cv::THRESH_OTSU);
    //闭运算，去除黑点
    cv::Mat element = getStructuringElement(cv::MORPH_RECT, cv::Size(5, 5));
    cv::Mat result;
    cv::Mat templ_bin_morph;
    cv::morphologyEx(templ_bin, templ_bin_morph, cv::MORPH_CLOSE, element);
    cv::imshow("templ_bin", templ_bin);
    cv::imwrite("./images/templ/templ_bin.bmp", templ_bin);
    cv::imshow("templ_bin_morph", templ_bin_morph); //形态学效果不如意
//    cv::waitKey(0);

    //templ_bin_mask
    cv::Mat templ_bin_mask;
    vector<vector<cv::Point>> contours;
    vector<cv::Vec4i> hierarchy;
    cv::findContours(templ_bin, contours, hierarchy, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);
    cv::Mat templ_bin_contours = cv::Mat::zeros(templ_bin.size(), CV_8UC3);  // 彩色轮廓使用3通道 CV_8UC3
    for (size_t i = 0; i < contours.size(); i++)
    {
        cv::drawContours(templ_bin_contours, contours, (int)i, cv::Scalar(255, 255, 255), -1, 8, hierarchy);
    }
    cv::imshow("templ_bin_contours", templ_bin_contours);
    cv::cvtColor(templ_bin_contours, templ_bin_mask, cv::COLOR_BGR2GRAY);
    cv::imshow("templ_bin_mask", templ_bin_mask);
    cv::imwrite("./images/templ/templ_bin_mask.bmp", templ_bin_mask);
    cv::waitKey(0);

    Utils utils;
    cv::Rect rect_matchResult = utils.getMatchResultROI(mat_logo, templ);
    cv::Mat matchResult = mat_logo(rect_matchResult);
    bool isAbnormal = utils.inspectLogo(matchResult, templ, templ_bin, templ_bin_mask, 200, 6);
    std::cout << "isAbnormal: " << isAbnormal << std::endl;
}

//抠logo作为模板
void create_templ_logo()
{
    QString filePath = "D:/Datasets/bdj/20220802-cam1-dataset/0027_1.bmp";
    cv::Mat src = cv::imread(filePath.toStdString(), cv::IMREAD_GRAYSCALE);
    cv::Mat templ = src(cv::Rect(1725, 1313, 134, 470));
//    cv::rotate(templ, templ, cv::ROTATE_90_CLOCKWISE);
    cv::imwrite("./images/templ/0027_1.bmp", templ);
}

//测试logo检测
//int main(int argc, char *argv[])
//{
//    create_templ_logo();
//    test_logo();
//    return 0;
//}


cv::Rect getBBox(const std::vector<int> &x_vec, const std::vector<int> &y_vec)
{
    int x_min = *min_element(x_vec.begin(), x_vec.end());
    int x_max = *max_element(x_vec.begin(), x_vec.end());
    int y_min = *min_element(y_vec.begin(), y_vec.end());;
    int y_max = *max_element(y_vec.begin(), y_vec.end());;
    return cv::Rect(x_min, y_min, x_max - x_min, y_max - y_min);
}


void func()
{
    //加载源图像
    //0008_1 0026_1 0027_1 0030_1 (0035_1 0036_1 0044_1) 0045_1 0048_1 0057_1 0058_1 0061_1
//    cv::Mat src = cv::imread("D:/Datasets/bdj/20220802-cam1-dataset/0004_1.bmp", cv::IMREAD_GRAYSCALE);
    cv::Mat src = cv::imread("D:/Datasets/bdj/20220802-cam1-dataset/0004_1.bmp", cv::IMREAD_GRAYSCALE);

    //抠字符
//    cv::Mat _4 = src(cv::Rect(1920, 1530, 69, 42));
//    cv::rotate(_4, _4, cv::ROTATE_90_CLOCKWISE);
//    cv::imshow("_4", _4);
//    cv::imwrite("./images/templ/4.bmp", _4);
//    cv::waitKey(0);

    //芯片区域
//    cv::Mat chip_img = src(cv::Rect(1565, 800, 840, 1250));
    cv::Mat chip_img = src(cv::Rect(1945, 965, 490, 910));
    cv::rotate(chip_img, chip_img, cv::ROTATE_90_CLOCKWISE);
//    cv::imshow("chip_img", chip_img);
//    cv::waitKey(0);


    //模拟ppocr检测的文本框
    int pad = 10;
    cv::Rect bbox(1910 - pad, 1305 - pad, 86 + 2 * pad, 480 + 2 * pad);
    cv::Mat CX = src(bbox);
    cv::rotate(CX, CX, cv::ROTATE_90_CLOCKWISE);
//    cv::imshow("CX", CX);
//    cv::waitKey(0);

    cv::Mat CX_show;
    cv::cvtColor(CX, CX_show, cv::COLOR_GRAY2BGR);
    // 遍历字符串，在文本行进行模板匹配，显示每个字符匹配的结果 CXDB4CCAM
    QString str("CXDB4CCAM");
    int char_width = CX.cols / str.size();  // 单个字符的匹配范围
    int idx = 0; //待匹配的子区域起始索引
    Utils utils;

    for (auto c : str)
    {
        // 搜索范围，初始化为0，后续根据上一个匹配的位置确定
        int shift = idx;
        int char_rect_width = char_width * 1.5;
        if (idx + char_width * 1.5 > CX.cols)
        {
            char_rect_width = CX.cols - idx;
        }
        cv::Rect char_rect = cv::Rect(shift, 0, char_rect_width, CX.rows);
        cv::Mat char_roi = CX(char_rect);
//        cv::imshow("char_roi", char_roi);
//        cv::waitKey(0);
        cv::Mat templ = cv::imread(QString("./images/templ/%1.bmp").arg(c).toStdString(), cv::IMREAD_GRAYSCALE);
        cv::Mat matchROI;
        cv::matchTemplate(char_roi, templ, matchROI, cv::TM_SQDIFF_NORMED);
        cv::Rect rect = utils.getMatchResultROI(char_roi, templ);
        cv::Rect new_rect = cv::Rect(rect.x + shift, rect.y, rect.width, rect.height);
        cv::rectangle(CX_show, new_rect, cv::Scalar(0, 255, 0), 2);
        cv::Mat matchResult = char_roi(rect);

        //二值化
        cv::Mat templ_bin, templ_mask;
        cv::threshold(templ, templ_bin, 0, 255, cv::THRESH_OTSU);
//        cv::imshow("templ_bin", templ_bin);
//        cv::waitKey(0);

        //mask，外接轮廓
        cv::Mat templ_bin_mask;
        vector<vector<cv::Point>> contours;
        vector<cv::Vec4i> hierarchy;

        cv::findContours(templ_bin, contours, hierarchy, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);
        cv::Mat drawing = cv::Mat::zeros(templ_bin.size(), CV_8UC3);  // 彩色轮廓使用3通道 CV_8UC3
        for (size_t i = 0; i < contours.size(); i++)
        {
//            cv::Scalar color = cv::Scalar(rng.uniform(0, 255), rng.uniform(0, 255), rng.uniform(0, 255));
            cv::drawContours(drawing, contours, (int)i, cv::Scalar(0, 255, 0), -1, 8, hierarchy);
        }
//        cv::imshow("Contours", drawing);
//        cv::waitKey(0);


        utils.ocv(matchResult, templ, templ_bin, templ_bin); //字符验证

        idx = new_rect.x + new_rect.width;
        cv::destroyAllWindows();
    }
    cv::imshow("match result", CX_show);
    cv::waitKey(0);
}

//抠字符作为模板
void create_templ_ocr()
{
//    QString filePath = "D:/Datasets/bdj/20221019/09/cam1-20221019-115556-926.bmp";
//    cv::Mat char_roi = src(cv::Rect(1954, 1792, 69, 37)); //C
//    cv::Mat char_roi = src(cv::Rect(1954, 1739, 70, 36)); //X
//    cv::Mat char_roi = src(cv::Rect(1951, 1575, 69, 39)); //4
//    cv::Mat char_roi = src(cv::Rect(1951, 1523, 69, 35)); //A
//    cv::Mat char_roi = src(cv::Rect(1948, 1354, 69, 50)); //M
//    cv::Mat char_roi = src(cv::Rect(2079, 1740, 66, 36)); //J
//    cv::Mat char_roi = src(cv::Rect(2071, 1265, 69, 38)); //2

//    QString filePath = "D:/Datasets/bdj/20221019/07/cam1-20221019-114621-160.bmp";
//    cv::Mat char_roi = src(cv::Rect(2070, 1103, 68, 35)); //3
//    cv::Mat char_roi = src(cv::Rect(1959, 1573, 71, 35)); //5
//    cv::Mat char_roi = src(cv::Rect(1959, 1410, 69, 37)); //0

//    QString filePath = "D:/Datasets/bdj/20221019/03/cam1-20221019-115257-834.bmp";
//    cv::Mat char_roi = src(cv::Rect(2092, 1734, 69, 38)); //K

    QString filePath = "D:/Datasets/bdj/20221019/01/cam1-20221019-114835-041.bmp";
    cv::Mat src = cv::imread(filePath.toStdString(), cv::IMREAD_GRAYSCALE);
    cv::Mat char_roi = src(cv::Rect(1970, 1679, 69, 34)); //D
    cv::rotate(char_roi, char_roi, cv::ROTATE_90_CLOCKWISE);
    cv::imwrite("./images/templ_20221019/D.bmp", char_roi);
}




//1、CDeviceProcess获取图片，向mainWindow发送检测请求
//mainWindow作为客户端，向python端发送检测请求
//python端返回文本框结果（得有前缀区分其他信息），mainWindow作为客户端根据返回结果处理图片
//CDeviceProcess遍历，utils.ocv(img, templ, templ_bin)
//ocv检测结果需要显示吗？
//#include "Client.h"
//int main(int argc, char *argv[])
//{
////    create_templ(); //抠字符模板

//    QApplication a(argc, argv);
//    Client client;
//    client.connectServer("127.0.0.1", 9999);
//    client.execute();
//    return a.exec();
//}


//ppocr C++版本
//int main(int argc, char *argv[])
//{
//    //初始化ppocr
//    LpOCRResult lpocrresult;
//    OCRParameter parameter;
//    parameter.enable_mkldnn = true;
//    parameter.cpu_math_library_num_threads = 10;
//    parameter.max_side_len = 960;
//    parameter.det = true;
//    parameter.cls = true;
//    parameter.rec = true;

//    //V3 en
//    std::string model_path = "./models/en_v3/";
//    std::string cls_infer = model_path + "ch_ppocr_mobile_v2.0_cls_infer";
//    std::string rec_infer = model_path + "en_PP-OCRv3_rec_infer";
//    std::string det_infer = model_path + "en_PP-OCRv3_det_infer";
//    std::string ocrkeys = model_path + "en_dict.txt";

//    int* pEngine = Initialize(const_cast<char*>(det_infer.c_str()),
//                              const_cast<char*>(cls_infer.c_str()),
//                              const_cast<char*>(rec_infer.c_str()),
//                              const_cast<char*>(ocrkeys.c_str()),
//                              parameter);



//    //加载源图像
//    //0008_1 0026_1 0027_1 0030_1 (0035_1 0036_1 0044_1) 0045_1 0048_1 0057_1 0058_1 0061_1
//    cv::Mat src = cv::imread("D:/Datasets/bdj/20220802-cam1-dataset/0004_1.bmp", cv::IMREAD_GRAYSCALE);

//    //抠字符
//    //    cv::Mat _4 = src(cv::Rect(1920, 1530, 69, 42));
//    //    cv::rotate(_4, _4, cv::ROTATE_90_CLOCKWISE);
//    //    cv::imshow("_4", _4);
//    //    cv::imwrite("./images/templ/4.bmp", _4);
//    //    cv::waitKey(0);

//    //芯片区域
//    //    cv::Mat chip_img = src(cv::Rect(1565, 800, 840, 1250));
//    cv::Mat chip_img = src(cv::Rect(1945, 965, 490, 910));
//    cv::rotate(chip_img, chip_img, cv::ROTATE_90_CLOCKWISE);
//    //    cv::imshow("chip_img", chip_img);
//    //    cv::waitKey(0);


//    //====== ppocr Detect Mat ======
//    std::cout << "====== start: ppocr Detect Mat ======" << std::endl;
//    cv::Mat img = chip_img.clone();
//    //    cv::Mat img = cv::imread("./images/roi.bmp", cv::IMREAD_GRAYSCALE);
//    cv::cvtColor(img, img, cv::COLOR_GRAY2BGR);
//    cv::Mat img_vis = img.clone();
//    if (img.empty())
//    {
//        std::cout << "img emtpy" << std::endl;
//        return 0;
//    }
//    auto start = system_clock::now();
//    int cnt = DetectMat(pEngine, img, &lpocrresult); //耗时90ms左右；注意该函数会释放掉cv::Mat；要传入彩图，否则识别错误
//    if (cnt > 0)
//    {
//        for (int i = cnt - 1; i >= 0; i--)
//        {
//            //            std::vector<int> x_vec, y_vec;
//            for (int j = 0; j < 4; j++)
//            {

//                std::cout << lpocrresult->pOCRText[i].points[j].x << ", ";
//                std::cout << lpocrresult->pOCRText[i].points[j].y << ", ";
//                //                x_vec.push_back(lpocrresult->pOCRText[i].points[j].x);
//                //                y_vec.push_back(lpocrresult->pOCRText[i].points[j].y);
//            }
//            //            cv::Rect bbox = getBBox(x_vec, y_vec);
//            //            cv::rectangle(img_vis, bbox, cv::Scalar(0, 255, 0), 2, 8);

//            wstring text = (WCHAR*)(lpocrresult->pOCRText[i].ptext);
//            std::wcout << text << std::endl;
//        }
//    }
//    FreeDetectResult(lpocrresult);
//    auto end = system_clock::now();
//    auto duration =	duration_cast<milliseconds>(end - start);
//    std::cout << duration.count() * 0.001 << "s" << endl;
//    cv::imshow("img_vis", img_vis);
//    cv::waitKey(0);

//    try
//    {
//        FreeEngine(pEngine);
//    }
//    catch (const std::exception& e)
//    {
//        std::cout << e.what();
//    }
//    std::cout << "====== end: ppocr Detect Mat ======" << std::endl;

//    //在待检测图像上显示文本框


//    //====== ppocr Detect Mat ======



//    //模拟ppocr检测的文本框
//    int pad = 10;
//    cv::Rect bbox(1910 - pad, 1305 - pad, 86 + 2 * pad, 480 + 2 * pad);
//    cv::Mat CX = src(bbox);
//    cv::rotate(CX, CX, cv::ROTATE_90_CLOCKWISE);
//    //    cv::imshow("CX", CX);
//    //    cv::waitKey(0);

//    cv::Mat CX_show;
//    cv::cvtColor(CX, CX_show, cv::COLOR_GRAY2BGR);
//    // //遍历字符串，在文本行进行模板匹配，显示每个字符匹配的结果 CXDB4CCAM
//    QString str("CXDB4CCAM");
//    int char_width = CX.cols / str.size();  // 单个字符的匹配范围
//    int idx = 0; //待匹配的子区域起始索引
//    Utils utils;

//    for (auto c : str)
//    {
//        // 搜索范围，初始化为0，后续根据上一个匹配的位置确定
//        int shift = idx;
//        int char_rect_width = char_width * 1.5;
//        if (idx + char_width * 1.5 > CX.cols)
//        {
//            char_rect_width = CX.cols - idx;
//        }
//        cv::Rect char_rect = cv::Rect(shift, 0, char_rect_width, CX.rows);
//        cv::Mat char_roi = CX(char_rect);
//        //        cv::imshow("char_roi", char_roi);
//        //        cv::waitKey(0);
//        cv::Mat templ = cv::imread(QString("./images/templ/%1.bmp").arg(c).toStdString(), cv::IMREAD_GRAYSCALE);
//        cv::Mat matchROI;
//        cv::matchTemplate(char_roi, templ, matchROI, cv::TM_SQDIFF_NORMED);
//        cv::Rect rect = utils.getMatchResultROI(char_roi, templ);
//        cv::Rect new_rect = cv::Rect(rect.x + shift, rect.y, rect.width, rect.height);
//        cv::rectangle(CX_show, new_rect, cv::Scalar(0, 255, 0), 2);
//        cv::Mat matchResult = char_roi(rect);

//        //二值化
//        cv::Mat templ_bin, templ_mask;
//        cv::threshold(templ, templ_bin, 0, 255, cv::THRESH_OTSU);
//        //        cv::imshow("templ_bin", templ_bin);
//        //        cv::waitKey(0);

//        //mask，外接轮廓
//        cv::Mat templ_bin_mask;
//        vector<vector<cv::Point>> contours;
//        vector<cv::Vec4i> hierarchy;

//        cv::findContours(templ_bin, contours, hierarchy, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);
//        cv::Mat drawing = cv::Mat::zeros(templ_bin.size(), CV_8UC3);  // 彩色轮廓使用3通道 CV_8UC3
//        for (size_t i = 0; i < contours.size(); i++)
//        {
//            //            cv::Scalar color = cv::Scalar(rng.uniform(0, 255), rng.uniform(0, 255), rng.uniform(0, 255));
//            cv::drawContours(drawing, contours, (int)i, cv::Scalar(0, 255, 0), -1, 8, hierarchy);
//        }
//        //        cv::imshow("Contours", drawing);
//        //        cv::waitKey(0);


//        utils.ocv(matchResult, templ, templ_bin, templ_bin); //字符验证

//        idx = new_rect.x + new_rect.width;
//        cv::destroyAllWindows();
//    }
//    cv::imshow("match result", CX_show);
//    cv::waitKey(0);
//}






//int main(int argc, char *argv[])
//{
//    Utils utils;
//    QString dirPath = "./images/20220802-cam1-dataset/";
//    QDir dir(dirPath);
//    if (!dir.exists())
//        return -1;
//    QStringList fileNames = dir.entryList(QDir::Files);
//    // 遍历测试
////    for (auto fileName : fileNames)
////    {
////        QString filePath = dirPath + fileName;
////        std::cout << filePath.toStdString() << std::endl;
////        cv::Mat img = cv::imread(filePath.toStdString(), cv::IMREAD_GRAYSCALE);
//////        cv::Mat img = cv::imread("./images/20220802-cam1-dataset/0006_6.bmp", cv::IMREAD_GRAYSCALE);
////        cv::Rect roi(1595, 820, 830, 1245);
////        cv::Mat chip_roi(img(roi));
////        int nType = utils.getType(chip_roi);
////        std::cout << "[nType]: " << nType << "\n" << std::endl;
////    }

//    // 单独测试
//    cv::Mat img = cv::imread("./images/20220802-cam1-dataset/0011_1.bmp", cv::IMREAD_GRAYSCALE);
////    cv::Rect roi(1495, 820, 830, 1245); // 偏差
//    cv::Rect roi(1595, 820, 830, 1245);
//    cv::Mat chip_roi(img(roi));
//    int nType = utils.getTypeTest(chip_roi);
//    std::cout << "[nType]: " << nType << "\n" << std::endl;

////    utils.binarizationWithSlider(chip_roi);
//}

//测试遮挡logo
//int main(int argc, char *argv[])
//{
//    Utils utils;

//    QString dirPath = "./images/test2/";
//    QDir dir(dirPath);
//    if (!dir.exists())
//        return -1;
//    QStringList fileNames = dir.entryList(QDir::Files);
//    // 遍历测试
//    for (auto fileName : fileNames)
//    {
//        QString filePath = dirPath + fileName;
//        std::cout << filePath.toStdString() << std::endl;
//        cv::Mat img = cv::imread(filePath.toStdString(), cv::IMREAD_GRAYSCALE);
//        cv::Rect chip_roi(1595, 820, 830, 1245); //chip_roi
//        cv::Mat chip(img(chip_roi));
//        cv::Rect logo_roi(1600, 1200, 335, 675); //框选得到
//        cv::Mat logo = img(logo_roi); //logo
//        // 获取缺陷类型
//        int nType = utils.getType(chip);
//        std::cout << "[nType]: " << nType << std::endl;
//        if (nType == 1)
//        {
//            //检测logo
//            bool isAbnormal = utils.inspectLogo(logo);
//            if (isAbnormal)
//            {
//                QString inspectResultSavePath = "./logoResult/";
//                cout << "Logo is abnormal" << endl;
//                if (!QDir().exists(inspectResultSavePath))
//                {
//                    QDir().mkpath(inspectResultSavePath);
//                }
//                cv::Rect matchResult_roi = utils.getMatchResultROI(logo, utils.templ);
//                cv::Rect match_roi = cv::Rect(logo_roi.x + matchResult_roi.x, logo_roi.y + matchResult_roi.y, matchResult_roi.width, matchResult_roi.height);
//                cv::cvtColor(img, img, cv::COLOR_GRAY2BGR);
//                cv::rectangle(img, match_roi, cv::Scalar(0, 0, 255), 2, 8);
//                QString inspectResultFilePath = inspectResultSavePath + fileName;
//                clock_t start = clock();
//                cv::imwrite(inspectResultFilePath.toStdString(), img);
//                clock_t end = clock();
//                std::cout << "elapsed_time: " << end - start << " ms" << std::endl << std::endl;
//            }
//        }
//    }
//}


