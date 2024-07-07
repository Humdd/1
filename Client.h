#ifndef CLIENT_H
#define CLIENT_H
#include <QObject>
#include <QTcpSocket>
#include <QHostAddress>
#include <QJsonDocument>
#include <QJsonArray>
#include "CProcess.h"
#include "utils.h"
//模拟mainWindow
class Client : public QObject
{
    Q_OBJECT
public:
    Client()
        : cProcess(new CProcess)
    {
        // 检测服务器是否回复了数据
        connect(m_tcp, &QTcpSocket::readyRead, [=]()
        {
            // 接收服务器发送的数据
            QByteArray recvMsg = m_tcp->readAll();
            qDebug() << QString("服务器Say: %1").arg(QString(recvMsg));

            if (recvMsg == "ok")
            {
                sendImage(m_text_roi);
            }
            else
            {//返回矩形框的消息
                auto _2DVec = parse2DArray(recvMsg);
                //按文本行进行匹配
//                QStringList text_list = {"CCDB5ABXM", "MJ"}; //01
//                QStringList text_list = {  "MXBA5BDCC", "JM"}; //01 - 变
//                QStringList text_list = {"CXDB4ABAM", "MK"}; //03
//                QStringList text_list = {  "MABA4BDCX", "KM"}; //03 - 变
//                QStringList text_list = {  "CADB5AB0M", "MA"}; //07
//                QStringList text_list = {"CXDB4ABAM", "MJ", "2223"}; //09
//                QStringList text_list = {  "MABA4BDXC", "JM"};

                QStringList text_list = {  "CXDB4CCAM", "MK"}; // 20220802 0004_1.bmp


                //绘制矩形框
                cv::Mat img_show;
                cv::cvtColor(m_text_roi, img_show, cv::COLOR_GRAY2BGR);
                for (int i = 0; i < _2DVec.size(); i++)
                {
                    auto bbox = _2DVec.at(i);
                    //待匹配矩形框
                    int pad = 10;
                    cv::Rect rect = cv::Rect(bbox[0] - pad, bbox[1] - pad, bbox[2] + 2 * pad, bbox[3] + 2 * pad);
                    cv::Mat line_img = m_text_roi(rect);
                    line_ocv(line_img, text_list.at(i));
                }
                for (const auto &subarr : _2DVec)
                {
                    //待匹配矩形框
                    cv::Rect rect = cv::Rect(subarr[0], subarr[1], subarr[2], subarr[3]);
                    cv::rectangle(img_show, rect, cv::Scalar(0, 255, 0), 2, 8);
                }
//                cv::imshow("img_show", img_show);
//                cv::waitKey(0);
            }
        });

        // 检测是否和服务器是否连接成功了
        connect(m_tcp, &QTcpSocket::connected, this, [=]()
        {
            qDebug() << "恭喜, 连接服务器成功!!!";
        });

        // 检测服务器是否和客户端断开了连接
        connect(m_tcp, &QTcpSocket::disconnected, this, [=]()
        {
            qDebug() << "服务器已经断开了连接, ...";
        });

        connect(cProcess, &CProcess::sendMsgToMainwindow, this, &Client::sendMsg);
        connect(cProcess, &CProcess::sendImageToMainwindow, this, &Client::sendImage);
    }

    void execute()
    {
        cProcess->callback();
    }

    //单字符匹配
    void char_ocv(const cv::Mat &char_img, const QString &ch)
    {

    }

    //行匹配
    void line_ocv(const cv::Mat &line_img, const QString &text_line)
    {
        cv::Mat img_vis;
        cv::cvtColor(line_img, img_vis, cv::COLOR_GRAY2BGR);
        //遍历字符串，在文本行进行模板匹配，显示每个字符匹配的结果 CXDB4CCAM
        int char_width = line_img.cols / text_line.size();  // 单个字符的匹配范围
        int idx = 0; //待匹配的子区域起始索引
        Utils utils;

        for (auto c : text_line)
        {
            std::cout << QString(c).toStdString() << std::endl;
            // 搜索范围，初始化为0，后续根据上一个匹配的位置确定
            int shift = idx;
            int char_rect_width = char_width * 1.5;
            if (idx + char_width * 1.5 > line_img.cols)
            {
                char_rect_width = line_img.cols - idx;
            }
            cv::Rect char_rect = cv::Rect(shift, 0, char_rect_width, line_img.rows);
            cv::Mat char_roi = line_img(char_rect);
#ifdef _DEBUG
            cv::imshow("char_roi", char_roi);
#endif
    //        cv::waitKey(0);
            //使用前需要判断templ是否存在
            cv::Mat templ = cv::imread(QString("./images/templ_20221019/%1.bmp").arg(c).toStdString(), cv::IMREAD_GRAYSCALE);
            if (templ.empty())
            {
                std::cout << "templ empty" << std::endl;
                return;
            }
            cv::Mat matchROI;
            cv::matchTemplate(char_roi, templ, matchROI, cv::TM_SQDIFF_NORMED);
            cv::Rect rect = utils.getMatchResultROI(char_roi, templ);
            cv::Rect new_rect = cv::Rect(rect.x + shift, rect.y, rect.width, rect.height);
//            cv::rectangle(img_vis, new_rect, cv::Scalar(0, 255, 0), 2);
            cv::Mat matchResult = char_roi(rect);

            //二值化
            cv::Mat templ_bin, templ_mask;
            cv::threshold(templ, templ_bin, 0, 255, cv::THRESH_OTSU);
    //        cv::imshow("templ_bin", templ_bin);
    //        cv::waitKey(0);

            //mask，外接轮廓
            cv::Mat templ_bin_mask;
            std::vector<std::vector<cv::Point>> contours;
            std::vector<cv::Vec4i> hierarchy;

            cv::findContours(templ_bin, contours, hierarchy, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);
            cv::Mat drawing = cv::Mat::zeros(templ_bin.size(), CV_8UC3);  // 彩色轮廓使用3通道 CV_8UC3
            for (size_t i = 0; i < contours.size(); i++)
            {
                cv::drawContours(drawing, contours, (int)i, cv::Scalar(0, 255, 0), -1, 8, hierarchy);
            }
    //        cv::imshow("Contours", drawing);
    //        cv::waitKey(0);

            bool isAbnormal = utils.ocv(matchResult, templ, templ_bin, templ_bin); //字符验证
            if (isAbnormal)
            {
                cv::rectangle(img_vis, new_rect, cv::Scalar(0, 0, 255), 2); //根据验证结果绘图
            }
            else
            {
                cv::rectangle(img_vis, new_rect, cv::Scalar(0, 255, 0), 2); //根据验证结果绘图
            }
            idx = new_rect.x + new_rect.width;
            cv::destroyAllWindows();
        }
        cv::imshow("match result", img_vis);
        cv::waitKey(0);
    }


    void connectServer(QString ip, int port)
    {
        // 连接服务器
        m_tcp->connectToHost(QHostAddress(ip), port);
        qDebug() << QString("connect server, address(%1, %2)").arg(ip, QString::number(port));
    }

    std::vector<std::vector<int> > parse2DArray(const QString &str)
    {
        std::vector<std::vector<int>> res;
        QJsonDocument jDoc = QJsonDocument::fromJson(str.toUtf8());
        QJsonArray jArray = jDoc.array();

        foreach (const auto &item, jArray)
        {
            std::vector<int> temp;
            auto subarr = item.toArray();
            foreach (const auto &subitem, subarr)
            {
                temp.push_back(subitem.toInt());
            }
            res.push_back(temp);
        }
        return res;
    }

private slots:
    void sendMsg(const QString &msg)
    {
        m_tcp->write(msg.toUtf8());
        qDebug() << QString("sendMsg: %1").arg(msg);
    }

    void sendImage(const cv::Mat &img)
    {
        m_text_roi = img.clone();
        // Conversion from Mat to QByteArray
        QByteArray qByteArray((char*)(img.data), img.total() * img.elemSize());
        m_tcp->write(qByteArray);
    }
private:
    QTcpSocket *m_tcp = new QTcpSocket(nullptr);
    CProcess *cProcess;
    cv::Mat m_text_roi;
};

#endif // CLIENT_H
