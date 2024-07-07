#ifndef CPROCESS_H
#define CPROCESS_H
#include <QObject>
#include "opencv2/opencv.hpp"

class CProcess : public QObject
{
    Q_OBJECT
public:
    CProcess()
    {
//        qRegisterMetaType("CProcess");
    }
    void callback()
    {
//        QString filePath = "D:/Datasets/bdj/20221019/01/cam1-20221019-114835-041.bmp";
//        QString filePath = "D:/Datasets/bdj/20221019/03/cam1-20221019-115257-834.bmp";
//        QString filePath = "D:/Datasets/bdj/20221019/07/cam1-20221019-114621-160.bmp";
//        QString filePath = "D:/Datasets/bdj/20221019/09/cam1-20221019-115556-926.bmp";

        QString filePath = "D:/Datasets/bdj/20220802-cam1-dataset/0004_1.bmp";
        cv::Mat src = cv::imread(filePath.toStdString(), cv::IMREAD_GRAYSCALE);
        cv::Mat chip_img = src(cv::Rect(1600, 830, 845, 1270));
        cv::rotate(chip_img, chip_img, cv::ROTATE_90_CLOCKWISE);
//        cv::Mat text_roi = src(cv::Rect(1900, 1045, 335, 865));
//        cv::Mat text_roi = src(cv::Rect(1900, 1305, 297, 587)); //20221019
        cv::Mat text_roi = src(cv::Rect(1944, 1291, 280, 532)); //20220802 0004_1.bmp
        m_text_roi = text_roi.clone();
        cv::rotate(text_roi, text_roi, cv::ROTATE_90_CLOCKWISE);
//        cv::imshow("text_roi", text_roi);
//        cv::waitKey(0);

        //send signal
        int totalBytes = text_roi.total() * text_roi.elemSize();
        QString msg = QString("ppocr|%1|%2|%3").arg(QString::number(totalBytes),
                                                    QString::number(text_roi.rows),
                                                    QString::number(text_roi.cols));
        emit sendMsgToMainwindow(msg);
        emit sendImageToMainwindow(text_roi);
    }
signals:
    void sendMsgToMainwindow(const QString &msg);
    void sendImageToMainwindow(const cv::Mat &img);
private:
    cv::Mat m_text_roi;
};

#endif // CPROCESS_H
